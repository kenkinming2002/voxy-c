#include "database.h"

#include "chunk/seed.h"
#include "group.h"

#include <voxy/config.h>

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcore/log.h>
#include <libcore/fs.h>
#include <libcore/format.h>
#include <libcore/profile.h>
#include <libcore/unreachable.h>

#include <liburing.h>

#include <stb_ds.h>

#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#define CHUNK_DATABASE_LOAD_LIMIT 512

struct voxy_block_group;

struct block_database_load_wrapper
{
  int fixed_file;

  char *path;
  struct voxy_block_group *block_group;
  struct iovec iovecs[2];

  bool done;
};

struct block_database_load_entry
{
  ivec3_t key;
  struct block_database_load_wrapper *value;
};

struct block_database_save_wrapper
{
  int fixed_file;

  char *dirs[3];
  char *path;
  struct iovec iovecs[2];

  bool done;
};

struct block_database_save_entry
{
  ivec3_t key;
  struct block_database_save_wrapper *value;
};

static struct io_uring ring;
static size_t fixed_file_bitmaps[CHUNK_DATABASE_LOAD_LIMIT / SIZE_WIDTH];

static char *directory;
static struct block_database_load_entry *load_entries;
static struct block_database_save_entry *save_entries;

#define block_group_dir0(directory, position) "%s/%d",            directory, position.x
#define block_group_dir1(directory, position) "%s/%d/%d/",        directory, position.x, position.y
#define block_group_dir2(directory, position) "%s/%d/%d/%d",      directory, position.x, position.y, position.z
#define block_group_file(directory, position) "%s/%d/%d/%d/data", directory, position.x, position.y, position.z

#define block_group_size (sizeof ((struct voxy_block_group *)0)->ids + sizeof ((struct voxy_block_group *)0)->lights)

void voxy_block_database_init(void)
{
  io_uring_queue_init(512, &ring, 0);
  io_uring_register_files_sparse(&ring, CHUNK_DATABASE_LOAD_LIMIT);
  memset(&fixed_file_bitmaps, 0, sizeof fixed_file_bitmaps);

  directory = aformat("%s/chunks/blocks", world_get_directory());
  if(mkdir_recursive(directory) != 0)
  {
    LOG_ERROR("Failed to create directory: %s", directory);
    exit(EXIT_FAILURE);
  }

  load_entries = NULL;
  save_entries = NULL;
}

enum tag
{
  TAG_LOAD_OPEN_DIRECT,
  TAG_LOAD_READV,
  TAG_LOAD_CLOSE_DIRECT,

  TAG_SAVE_MKDIR,

  TAG_SAVE_OPEN_DIRECT,
  TAG_SAVE_READV,
  TAG_SAVE_CLOSE_DIRECT,
};

#define TAG_MASK 0x7

#define tagged_ptr(ptr, tag) ((uintptr_t)(ptr) | (tag))

#define tagged_ptr_value(tagged_ptr) ((void *)((tagged_ptr) & ~TAG_MASK))
#define tagged_ptr_tag(tagged_ptr) (enum tag)((tagged_ptr) & TAG_MASK)

static int alloc_fixed_file(void)
{
  for(int i=0; i<CHUNK_DATABASE_LOAD_LIMIT/SIZE_WIDTH; ++i)
    if(fixed_file_bitmaps[i] != SIZE_MAX)
      for(int j=0; j<SIZE_WIDTH; ++j)
        if(!(fixed_file_bitmaps[i] & (size_t)((size_t)1 << j)))
        {
          fixed_file_bitmaps[i] |= (size_t)((size_t)1 << j);
          return i * SIZE_WIDTH + j;
        }

  return -1;
}

static void free_fixed_file(int fixed_file)
{
  int i = fixed_file / SIZE_WIDTH;
  int j = fixed_file % SIZE_WIDTH;
  fixed_file_bitmaps[i] &= ~(size_t)((size_t)1 << j);
}

static void io_uring_sq_ensure_space(struct io_uring *ring, unsigned n)
{
  if(io_uring_sq_space_left(ring) < n)
    io_uring_submit(ring);
}

struct block_group_future voxy_block_database_load(ivec3_t position)
{
  profile_scope;

  ptrdiff_t i = hmgeti(load_entries, position);
  if(i == -1)
  {
    int fixed_file = alloc_fixed_file();
    if(fixed_file == -1)
      return block_group_future_pending;

    struct block_database_load_wrapper *wrapper = malloc(sizeof *wrapper);

    wrapper->fixed_file = fixed_file;

    wrapper->path = aformat(block_group_file(directory, position));

    wrapper->block_group = voxy_block_group_create();
    wrapper->block_group->disk_dirty = false;
    wrapper->block_group->network_dirty = true;

    wrapper->iovecs[0].iov_base = wrapper->block_group->ids;
    wrapper->iovecs[0].iov_len = sizeof wrapper->block_group->ids;
    wrapper->iovecs[1].iov_base = wrapper->block_group->lights;
    wrapper->iovecs[1].iov_len = sizeof wrapper->block_group->lights;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&ring, 3);

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_RDONLY, 0, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_readv(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_READV));

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_CLOSE_DIRECT));

    hmput(load_entries, position, wrapper);
    return block_group_future_pending;
  }

  struct block_database_load_wrapper *wrapper = load_entries[i].value;
  if(!wrapper->done)
    return block_group_future_pending;

  struct voxy_block_group *block_group = wrapper->block_group;
  hmdel(load_entries, position);
  free(wrapper);
  return block_group_future_ready(block_group);
}

struct unit_future voxy_block_database_save(ivec3_t position, struct voxy_block_group *block_group)
{
  profile_scope;

  ptrdiff_t i = hmgeti(save_entries, position);
  if(i == -1)
  {
    int fixed_file = alloc_fixed_file();
    if(fixed_file == -1)
      return unit_future_pending;

    struct block_database_save_wrapper *wrapper = malloc(sizeof *wrapper);

    wrapper->fixed_file = fixed_file;

    wrapper->dirs[0] = aformat(block_group_dir0(directory, position));
    wrapper->dirs[1] = aformat(block_group_dir1(directory, position));
    wrapper->dirs[2] = aformat(block_group_dir2(directory, position));

    wrapper->path = aformat(block_group_file(directory, position));

    wrapper->iovecs[0].iov_base = block_group->ids;
    wrapper->iovecs[0].iov_len = sizeof block_group->ids;
    wrapper->iovecs[1].iov_base = block_group->lights;
    wrapper->iovecs[1].iov_len = sizeof block_group->lights;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&ring, 6);

    for(unsigned i=0; i<3; ++i)
    {
      sqe = io_uring_get_sqe(&ring);
      io_uring_prep_mkdir(sqe, wrapper->dirs[i], 0755);
      io_uring_sqe_set_flags(sqe, IOSQE_IO_HARDLINK);
      io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_MKDIR));
    }

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_WRONLY | O_CREAT | O_TRUNC, 0644, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_writev(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_READV));

    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_CLOSE_DIRECT));

    hmput(save_entries, position, wrapper);
    return unit_future_pending;
  }

  struct block_database_save_wrapper *wrapper = save_entries[i].value;
  if(!wrapper->done)
    return unit_future_pending;

  hmdel(save_entries, position);
  free(wrapper);
  return unit_future_ready;
}

void voxy_block_database_update(void)
{
  profile_scope;

  io_uring_submit(&ring);

  struct io_uring_cqe *cqe;
  unsigned head;
  unsigned count = 0;
  io_uring_for_each_cqe(&ring, head, cqe)
  {
    uintptr_t tagged_ptr = io_uring_cqe_get_data64(cqe);
    switch(tagged_ptr_tag(tagged_ptr))
    {
    case TAG_LOAD_OPEN_DIRECT:
      {
        struct block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ENOENT)
        {
          free_fixed_file(wrapper->fixed_file);
          free(wrapper->path);
          voxy_block_group_destroy(wrapper->block_group);

          wrapper->path = NULL;
          wrapper->block_group = NULL;
          wrapper->done = true;
          break;
        }

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to open file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }
      }
      break;
    case TAG_LOAD_READV:
      {
        struct block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to read from file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        if(cqe->res != block_group_size)
        {
          LOG_ERROR("Partial read from file: %s: expected %zu bytes, got %d bytes", wrapper->path, block_group_size, cqe->res);
          break;
        }
      }
      break;
    case TAG_LOAD_CLOSE_DIRECT:
      {
        struct block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(wrapper->fixed_file);
        free(wrapper->path);

        wrapper->path = NULL;
        wrapper->done = true;
        break;
      }
      break;
    case TAG_SAVE_MKDIR:
      {
        struct block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res == -EEXIST)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to create parent directory: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }
      }
      break;
    case TAG_SAVE_OPEN_DIRECT:
      {
        struct block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to open file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }
      }
      break;
    case TAG_SAVE_READV:
      {
        struct block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to write to file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        if(cqe->res != block_group_size)
        {
          LOG_ERROR("Partial write to file: %s: expected %zu bytes, got %d bytes", wrapper->path, block_group_size, cqe->res);
          break;
        }
      }
      break;
    case TAG_SAVE_CLOSE_DIRECT:
      {
        struct block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(wrapper->fixed_file);
        for(unsigned i=0; i<3; ++i) free(wrapper->dirs[i]);
        free(wrapper->path);

        for(unsigned i=0; i<3; ++i) wrapper->dirs[i] = NULL;
        wrapper->path = NULL;
        wrapper->done = true;
        break;
      }
      break;
    }

    ++count;
  }
  io_uring_cq_advance(&ring, count);
}
