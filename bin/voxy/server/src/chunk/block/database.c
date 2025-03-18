#include "database.h"

#include "group.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcore/log.h>
#include <libcore/fs.h>
#include <libcore/format.h>
#include <libcore/profile.h>
#include <libcore/unreachable.h>

#include <string.h>
#include <errno.h>

#include <fcntl.h>
#include <unistd.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_KEY_TYPE ivec3_t

#define SC_HASH_TABLE_PREFIX voxy_block_database_load_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_block_database_load_wrapper
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE

#define SC_HASH_TABLE_PREFIX voxy_block_database_save_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_block_database_save_wrapper
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE

#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

ivec3_t voxy_block_database_load_wrapper_key(struct voxy_block_database_load_wrapper *wrapper) { return wrapper->position; }
size_t voxy_block_database_load_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_block_database_load_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_block_database_load_wrapper_dispose(struct voxy_block_database_load_wrapper *wrapper)
{
  if(wrapper->path) free(wrapper->path);
  if(wrapper->block_group) voxy_block_group_destroy(wrapper->block_group);
  free(wrapper);
}

ivec3_t voxy_block_database_save_wrapper_key(struct voxy_block_database_save_wrapper *wrapper) { return wrapper->position; }
size_t voxy_block_database_save_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_block_database_save_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_block_database_save_wrapper_dispose(struct voxy_block_database_save_wrapper *wrapper)
{
  for(unsigned i=0; i<3; ++i) if(wrapper->dirs[i]) free(wrapper->dirs[i]);
  if(wrapper->path) free(wrapper->path);
  free(wrapper);
}

#define block_group_dir0(directory, position) "%s/%d",            directory, position.x
#define block_group_dir1(directory, position) "%s/%d/%d/",        directory, position.x, position.y
#define block_group_dir2(directory, position) "%s/%d/%d/%d",      directory, position.x, position.y, position.z
#define block_group_file(directory, position) "%s/%d/%d/%d/data", directory, position.x, position.y, position.z

#define block_group_size (sizeof ((struct voxy_block_group *)0)->ids + sizeof ((struct voxy_block_group *)0)->light_levels)

void voxy_block_database_init(struct voxy_block_database *block_database, const char *world_directory)
{
  io_uring_queue_init(512, &block_database->ring, 0);
  io_uring_register_files_sparse(&block_database->ring, CHUNK_DATABASE_LOAD_LIMIT);
  memset(&block_database->fixed_file_bitmaps, 0, sizeof block_database->fixed_file_bitmaps);

  block_database->directory = aformat("%s/chunks/blocks", world_directory);
  if(mkdir_recursive(block_database->directory) != 0)
  {
    LOG_ERROR("Failed to create directory: %s", block_database->directory);
    exit(EXIT_FAILURE);
  }

  voxy_block_database_load_wrapper_hash_table_init(&block_database->load_wrappers);
  voxy_block_database_save_wrapper_hash_table_init(&block_database->save_wrappers);
}

void voxy_block_database_fini(struct voxy_block_database *block_database)
{
  io_uring_register_sync_cancel(&block_database->ring, &(struct io_uring_sync_cancel_reg){ .flags = IORING_ASYNC_CANCEL_ANY, .timeout = { .tv_sec = -1, .tv_nsec = -1 } });
  io_uring_queue_exit(&block_database->ring);

  free(block_database->directory);
  voxy_block_database_load_wrapper_hash_table_dispose(&block_database->load_wrappers);
  voxy_block_database_save_wrapper_hash_table_dispose(&block_database->save_wrappers);
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

static int alloc_fixed_file(struct voxy_block_database *block_database)
{
  for(int i=0; i<CHUNK_DATABASE_LOAD_LIMIT/SIZE_WIDTH; ++i)
    if(block_database->fixed_file_bitmaps[i] != SIZE_MAX)
      for(int j=0; j<SIZE_WIDTH; ++j)
        if(!(block_database->fixed_file_bitmaps[i] & (size_t)((size_t)1 << j)))
        {
          block_database->fixed_file_bitmaps[i] |= (size_t)((size_t)1 << j);
          return i * SIZE_WIDTH + j;
        }

  return -1;
}

static void free_fixed_file(struct voxy_block_database *block_database, int fixed_file)
{
  int i = fixed_file / SIZE_WIDTH;
  int j = fixed_file % SIZE_WIDTH;
  block_database->fixed_file_bitmaps[i] &= ~(size_t)((size_t)1 << j);
}

static void io_uring_sq_ensure_space(struct io_uring *ring, unsigned n)
{
  if(io_uring_sq_space_left(ring) < n)
    io_uring_submit(ring);
}

struct block_group_future voxy_block_database_load(struct voxy_block_database *block_database, ivec3_t position)
{
  profile_scope;

  struct voxy_block_database_load_wrapper *wrapper;
  if(!(wrapper = voxy_block_database_load_wrapper_hash_table_lookup(&block_database->load_wrappers, position)))
  {
    int fixed_file = alloc_fixed_file(block_database);
    if(fixed_file == -1)
      return block_group_future_pending;

    wrapper = malloc(sizeof *wrapper);

    wrapper->position = position;

    wrapper->fixed_file = fixed_file;

    wrapper->path = aformat(block_group_file(block_database->directory, position));

    wrapper->block_group = voxy_block_group_create();
    wrapper->block_group->position = wrapper->position;
    wrapper->block_group->disk_dirty = false;
    wrapper->block_group->network_dirty = true;

    wrapper->iovecs[0].iov_base = wrapper->block_group->ids;
    wrapper->iovecs[0].iov_len = sizeof wrapper->block_group->ids;
    wrapper->iovecs[1].iov_base = wrapper->block_group->light_levels;
    wrapper->iovecs[1].iov_len = sizeof wrapper->block_group->light_levels;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&block_database->ring, 3);

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_RDONLY, 0, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_readv(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_READV));

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_CLOSE_DIRECT));

    voxy_block_database_load_wrapper_hash_table_insert(&block_database->load_wrappers, wrapper);

    return block_group_future_pending;
  }

  if(!wrapper->done)
    return block_group_future_pending;

  struct voxy_block_group *block_group = wrapper->block_group;
  voxy_block_database_load_wrapper_hash_table_remove(&block_database->load_wrappers, wrapper->position);
  free(wrapper);
  return block_group_future_ready(block_group);
}

struct unit_future voxy_block_database_save(struct voxy_block_database *block_database, struct voxy_block_group *block_group)
{
  profile_scope;

  struct voxy_block_database_save_wrapper *wrapper;
  if(!(wrapper = voxy_block_database_save_wrapper_hash_table_lookup(&block_database->save_wrappers, block_group->position)))
  {
    int fixed_file = alloc_fixed_file(block_database);
    if(fixed_file == -1)
      return unit_future_pending;

    wrapper = malloc(sizeof *wrapper);

    wrapper->position = block_group->position;

    wrapper->fixed_file = fixed_file;

    wrapper->dirs[0] = aformat(block_group_dir0(block_database->directory, block_group->position));
    wrapper->dirs[1] = aformat(block_group_dir1(block_database->directory, block_group->position));
    wrapper->dirs[2] = aformat(block_group_dir2(block_database->directory, block_group->position));

    wrapper->path = aformat(block_group_file(block_database->directory, block_group->position));

    wrapper->iovecs[0].iov_base = block_group->ids;
    wrapper->iovecs[0].iov_len = sizeof block_group->ids;
    wrapper->iovecs[1].iov_base = block_group->light_levels;
    wrapper->iovecs[1].iov_len = sizeof block_group->light_levels;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&block_database->ring, 6);

    for(unsigned i=0; i<3; ++i)
    {
      sqe = io_uring_get_sqe(&block_database->ring);
      io_uring_prep_mkdir(sqe, wrapper->dirs[i], 0755);
      io_uring_sqe_set_flags(sqe, IOSQE_IO_HARDLINK);
      io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_MKDIR));
    }

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_WRONLY | O_CREAT | O_TRUNC, 0644, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_writev(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_READV));

    sqe = io_uring_get_sqe(&block_database->ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_CLOSE_DIRECT));

    voxy_block_database_save_wrapper_hash_table_insert(&block_database->save_wrappers, wrapper);

    return unit_future_pending;
  }

  if(!wrapper->done)
    return unit_future_pending;

  voxy_block_database_save_wrapper_hash_table_remove(&block_database->save_wrappers, wrapper->position);
  free(wrapper);
  return unit_future_ready;
}

void voxy_block_database_update(struct voxy_block_database *block_database)
{
  profile_scope;

  io_uring_submit(&block_database->ring);

  struct io_uring_cqe *cqe;
  unsigned head;
  unsigned count = 0;
  io_uring_for_each_cqe(&block_database->ring, head, cqe)
  {
    uintptr_t tagged_ptr = io_uring_cqe_get_data64(cqe);
    switch(tagged_ptr_tag(tagged_ptr))
    {
    case TAG_LOAD_OPEN_DIRECT:
      {
        struct voxy_block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ENOENT)
        {
          free_fixed_file(block_database, wrapper->fixed_file);
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
        struct voxy_block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

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
        struct voxy_block_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(block_database, wrapper->fixed_file);
        free(wrapper->path);

        wrapper->path = NULL;
        wrapper->done = true;
        break;
      }
      break;
    case TAG_SAVE_MKDIR:
      {
        struct voxy_block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

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
        struct voxy_block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to open file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }
      }
      break;
    case TAG_SAVE_READV:
      {
        struct voxy_block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

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
        struct voxy_block_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(block_database, wrapper->fixed_file);
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
  io_uring_cq_advance(&block_database->ring, count);
}
