#include "database.h"

#include "chunk.h"

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

#define SC_HASH_TABLE_PREFIX voxy_chunk_database_load_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_database_load_wrapper
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE

#define SC_HASH_TABLE_PREFIX voxy_chunk_database_save_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_database_save_wrapper
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE

#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

ivec3_t voxy_chunk_database_load_wrapper_key(struct voxy_chunk_database_load_wrapper *wrapper) { return wrapper->position; }
size_t voxy_chunk_database_load_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_chunk_database_load_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_chunk_database_load_wrapper_dispose(struct voxy_chunk_database_load_wrapper *wrapper)
{
  if(wrapper->path) free(wrapper->path);
  if(wrapper->chunk) voxy_chunk_destroy(wrapper->chunk);
  free(wrapper);
}

ivec3_t voxy_chunk_database_save_wrapper_key(struct voxy_chunk_database_save_wrapper *wrapper) { return wrapper->position; }
size_t voxy_chunk_database_save_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_chunk_database_save_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_chunk_database_save_wrapper_dispose(struct voxy_chunk_database_save_wrapper *wrapper)
{
  for(unsigned i=0; i<3; ++i) if(wrapper->dirs[i]) free(wrapper->dirs[i]);
  if(wrapper->path) free(wrapper->path);
  free(wrapper);
}

#define chunk_dir0(directory, position) "%s/%d",            directory, position.x
#define chunk_dir1(directory, position) "%s/%d/%d/",        directory, position.x, position.y
#define chunk_dir2(directory, position) "%s/%d/%d/%d",      directory, position.x, position.y, position.z
#define chunk_file(directory, position) "%s/%d/%d/%d/data", directory, position.x, position.y, position.z

#define chunk_size (sizeof ((struct voxy_chunk *)0)->block_ids + sizeof ((struct voxy_chunk *)0)->block_light_levels)

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database, const char *world_directory)
{
  io_uring_queue_init(512, &chunk_database->ring, 0);
  io_uring_register_files_sparse(&chunk_database->ring, CHUNK_DATABASE_LOAD_LIMIT);
  memset(&chunk_database->fixed_file_bitmaps, 0, sizeof chunk_database->fixed_file_bitmaps);

  chunk_database->directory = aformat("%s/chunks", world_directory);
  if(mkdir_recursive(tformat("%s/chunks", world_directory)) != 0)
  {
    LOG_ERROR("Failed to create directory: %s", chunk_database->directory);
    exit(EXIT_FAILURE);
  }

  voxy_chunk_database_load_wrapper_hash_table_init(&chunk_database->load_wrappers);
  voxy_chunk_database_save_wrapper_hash_table_init(&chunk_database->save_wrappers);
}

void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database)
{
  io_uring_register_sync_cancel(&chunk_database->ring, &(struct io_uring_sync_cancel_reg){ .flags = IORING_ASYNC_CANCEL_ANY, .timeout = { .tv_sec = -1, .tv_nsec = -1 } });
  io_uring_queue_exit(&chunk_database->ring);

  free(chunk_database->directory);
  voxy_chunk_database_load_wrapper_hash_table_dispose(&chunk_database->load_wrappers);
  voxy_chunk_database_save_wrapper_hash_table_dispose(&chunk_database->save_wrappers);
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

static int alloc_fixed_file(struct voxy_chunk_database *chunk_database)
{
  for(int i=0; i<CHUNK_DATABASE_LOAD_LIMIT/SIZE_WIDTH; ++i)
    if(chunk_database->fixed_file_bitmaps[i] != SIZE_MAX)
      for(int j=0; j<SIZE_WIDTH; ++j)
        if(!(chunk_database->fixed_file_bitmaps[i] & (size_t)((size_t)1 << j)))
        {
          chunk_database->fixed_file_bitmaps[i] |= (size_t)((size_t)1 << j);
          return i * SIZE_WIDTH + j;
        }

  return -1;
}

static void free_fixed_file(struct voxy_chunk_database *chunk_database, int fixed_file)
{
  int i = fixed_file / SIZE_WIDTH;
  int j = fixed_file % SIZE_WIDTH;
  chunk_database->fixed_file_bitmaps[i] &= ~(size_t)((size_t)1 << j);
}

static void io_uring_sq_ensure_space(struct io_uring *ring, unsigned n)
{
  if(io_uring_sq_space_left(ring) < n)
    io_uring_submit(ring);
}

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position)
{
  profile_scope;

  struct voxy_chunk_database_load_wrapper *wrapper;
  if(!(wrapper = voxy_chunk_database_load_wrapper_hash_table_lookup(&chunk_database->load_wrappers, position)))
  {
    int fixed_file = alloc_fixed_file(chunk_database);
    if(fixed_file == -1)
      return chunk_future_pending;

    wrapper = malloc(sizeof *wrapper);

    wrapper->position = position;

    wrapper->fixed_file = fixed_file;

    wrapper->path = aformat(chunk_file(chunk_database->directory, position));

    wrapper->chunk = voxy_chunk_create();
    wrapper->chunk->position = wrapper->position;
    wrapper->chunk->disk_dirty = false;
    wrapper->chunk->network_dirty = true;

    wrapper->iovecs[0].iov_base = wrapper->chunk->block_ids;
    wrapper->iovecs[0].iov_len = sizeof wrapper->chunk->block_ids;
    wrapper->iovecs[1].iov_base = wrapper->chunk->block_light_levels;
    wrapper->iovecs[1].iov_len = sizeof wrapper->chunk->block_light_levels;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&chunk_database->ring, 3);

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_RDONLY, 0, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_readv(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_READV));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_LOAD_CLOSE_DIRECT));

    voxy_chunk_database_load_wrapper_hash_table_insert(&chunk_database->load_wrappers, wrapper);

    return chunk_future_pending;
  }

  if(!wrapper->done)
    return chunk_future_pending;

  struct voxy_chunk *chunk = wrapper->chunk;
  voxy_chunk_database_load_wrapper_hash_table_remove(&chunk_database->load_wrappers, wrapper->position);
  free(wrapper);
  return chunk_future_ready(chunk);
}

struct unit_future voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk)
{
  profile_scope;

  struct voxy_chunk_database_save_wrapper *wrapper;
  if(!(wrapper = voxy_chunk_database_save_wrapper_hash_table_lookup(&chunk_database->save_wrappers, chunk->position)))
  {
    int fixed_file = alloc_fixed_file(chunk_database);
    if(fixed_file == -1)
      return unit_future_pending;

    wrapper = malloc(sizeof *wrapper);

    wrapper->position = chunk->position;

    wrapper->fixed_file = fixed_file;

    wrapper->dirs[0] = aformat(chunk_dir0(chunk_database->directory, chunk->position));
    wrapper->dirs[1] = aformat(chunk_dir1(chunk_database->directory, chunk->position));
    wrapper->dirs[2] = aformat(chunk_dir2(chunk_database->directory, chunk->position));

    wrapper->path = aformat(chunk_file(chunk_database->directory, chunk->position));

    wrapper->iovecs[0].iov_base = chunk->block_ids;
    wrapper->iovecs[0].iov_len = sizeof chunk->block_ids;
    wrapper->iovecs[1].iov_base = chunk->block_light_levels;
    wrapper->iovecs[1].iov_len = sizeof chunk->block_light_levels;

    wrapper->done = false;

    struct io_uring_sqe *sqe;
    io_uring_sq_ensure_space(&chunk_database->ring, 6);

    for(unsigned i=0; i<3; ++i)
    {
      sqe = io_uring_get_sqe(&chunk_database->ring);
      io_uring_prep_mkdir(sqe, wrapper->dirs[i], 0755);
      io_uring_sqe_set_flags(sqe, IOSQE_IO_HARDLINK);
      io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_MKDIR));
    }

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_WRONLY | O_CREAT | O_TRUNC, 0644, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_writev(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_READV));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_SAVE_CLOSE_DIRECT));

    voxy_chunk_database_save_wrapper_hash_table_insert(&chunk_database->save_wrappers, wrapper);

    return unit_future_pending;
  }

  if(!wrapper->done)
    return unit_future_pending;

  voxy_chunk_database_save_wrapper_hash_table_remove(&chunk_database->save_wrappers, wrapper->position);
  free(wrapper);
  return unit_future_ready;
}

void voxy_chunk_database_update(struct voxy_chunk_database *chunk_database)
{
  profile_scope;

  io_uring_submit(&chunk_database->ring);

  struct io_uring_cqe *cqe;
  unsigned head;
  unsigned count = 0;
  io_uring_for_each_cqe(&chunk_database->ring, head, cqe)
  {
    uintptr_t tagged_ptr = io_uring_cqe_get_data64(cqe);
    switch(tagged_ptr_tag(tagged_ptr))
    {
    case TAG_LOAD_OPEN_DIRECT:
      {
        struct voxy_chunk_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ENOENT)
        {
          free_fixed_file(chunk_database, wrapper->fixed_file);
          free(wrapper->path);
          voxy_chunk_destroy(wrapper->chunk);

          wrapper->path = NULL;
          wrapper->chunk = NULL;
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
        struct voxy_chunk_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to read from file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        if(cqe->res != chunk_size)
        {
          LOG_ERROR("Partial read from file: %s: expected %zu bytes, got %d bytes", wrapper->path, chunk_size, cqe->res);
          break;
        }
      }
      break;
    case TAG_LOAD_CLOSE_DIRECT:
      {
        struct voxy_chunk_database_load_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(chunk_database, wrapper->fixed_file);
        free(wrapper->path);

        wrapper->path = NULL;
        wrapper->done = true;
        break;
      }
      break;
    case TAG_SAVE_MKDIR:
      {
        struct voxy_chunk_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

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
        struct voxy_chunk_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to open file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }
      }
      break;
    case TAG_SAVE_READV:
      {
        struct voxy_chunk_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to write to file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        if(cqe->res != chunk_size)
        {
          LOG_ERROR("Partial write to file: %s: expected %zu bytes, got %d bytes", wrapper->path, chunk_size, cqe->res);
          break;
        }
      }
      break;
    case TAG_SAVE_CLOSE_DIRECT:
      {
        struct voxy_chunk_database_save_wrapper *wrapper = tagged_ptr_value(tagged_ptr);

        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to close file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        free_fixed_file(chunk_database, wrapper->fixed_file);
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
  io_uring_cq_advance(&chunk_database->ring, count);
}
