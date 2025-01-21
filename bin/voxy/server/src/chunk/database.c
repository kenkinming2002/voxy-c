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

#define CHUNK_DATABASE_LOAD_LIMIT 512

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX voxy_chunk_database_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_database_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t voxy_chunk_database_wrapper_key(struct voxy_chunk_database_wrapper *wrapper) { return wrapper->position; }
size_t voxy_chunk_database_wrapper_hash(ivec3_t position) { return ivec3_hash(position); }
int voxy_chunk_database_wrapper_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void voxy_chunk_database_wrapper_dispose(struct voxy_chunk_database_wrapper *wrapper)
{
  if(wrapper->path) free(wrapper->path);
  if(wrapper->chunk) voxy_chunk_destroy(wrapper->chunk);
  free(wrapper);
}

#define chunk_dir(position) "world/chunks/%d/%d/%d", position.x, position.y, position.z
#define chunk_file(position) "world/chunks/%d/%d/%d/blocks", position.x, position.y, position.z

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database)
{
  io_uring_queue_init(CHUNK_DATABASE_LOAD_LIMIT * 3 + 1, &chunk_database->ring, 0);
  io_uring_register_files_sparse(&chunk_database->ring, CHUNK_DATABASE_LOAD_LIMIT);
  memset(&chunk_database->fixed_file_bitmaps, 0, sizeof chunk_database->fixed_file_bitmaps);
  voxy_chunk_database_wrapper_hash_table_init(&chunk_database->wrappers);
}

void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database)
{
  io_uring_register_sync_cancel(&chunk_database->ring, &(struct io_uring_sync_cancel_reg){ .flags = IORING_ASYNC_CANCEL_ANY, .timeout = { .tv_sec = -1, .tv_nsec = -1 } });
  io_uring_queue_exit(&chunk_database->ring);
  voxy_chunk_database_wrapper_hash_table_dispose(&chunk_database->wrappers);
}

#define TAG_OPEN_DIRECT 0x0
#define TAG_READV 0x1
#define TAG_CLOSE_DIRECT 0x2
#define TAG_MASK 0x3

#define tagged_ptr(ptr, tag) ((uintptr_t)(ptr) | (tag))

#define tagged_ptr_value(tagged_ptr) ((void *)((tagged_ptr) & ~TAG_MASK))
#define tagged_ptr_tag(tagged_ptr) ((tagged_ptr) & TAG_MASK)

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

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position)
{
  profile_scope;

  struct voxy_chunk_database_wrapper *wrapper;
  if(!(wrapper = voxy_chunk_database_wrapper_hash_table_lookup(&chunk_database->wrappers, position)))
  {
    int fixed_file = alloc_fixed_file(chunk_database);
    if(fixed_file == -1)
      return chunk_future_pending;

    wrapper = malloc(sizeof *wrapper);

    wrapper->position = position;

    wrapper->fixed_file = fixed_file;

    wrapper->path = aformat(chunk_file(position));

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

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_open_direct(sqe, wrapper->path, O_RDONLY, 0, wrapper->fixed_file);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_OPEN_DIRECT));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_readv(sqe, wrapper->fixed_file, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
    io_uring_sqe_set_flags(sqe, IOSQE_IO_LINK | IOSQE_FIXED_FILE);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_READV));

    sqe = io_uring_get_sqe(&chunk_database->ring);
    io_uring_prep_close_direct(sqe, wrapper->fixed_file);
    io_uring_sqe_set_data64(sqe, tagged_ptr(wrapper, TAG_CLOSE_DIRECT));

    io_uring_submit(&chunk_database->ring);

    voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wrappers, wrapper);

    return chunk_future_pending;
  }

  if(!wrapper->done)
    return chunk_future_pending;

  struct voxy_chunk *chunk = wrapper->chunk;
  voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wrappers, wrapper->position);
  free(wrapper);

  if(!chunk) return chunk_future_reject;
  return chunk_future_ready(chunk);
}

int voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk)
{
  const char *dir = tformat(chunk_dir(chunk->position));
  if(mkdir_recursive(dir) != 0)
    goto err;

  const char *file = tformat(chunk_file(chunk->position));

  libserde_serializer_t serializer = libserde_serializer_create(file);
  if(!serializer)
    goto err;

  libserde_serializer_try_write(serializer, chunk->block_ids, err_serializer_destroy);
  libserde_serializer_try_write(serializer, chunk->block_light_levels, err_serializer_destroy);
  libserde_serializer_try_write_checksum(serializer, err_serializer_destroy);
  libserde_serializer_destroy(serializer);

  return 0;

err_serializer_destroy:
  libserde_serializer_destroy(serializer);
err:
  return -1;
}

void voxy_chunk_database_update(struct voxy_chunk_database *chunk_database)
{
  profile_scope;

  struct io_uring_cqe *cqe;
  while(io_uring_peek_cqe(&chunk_database->ring, &cqe) == 0)
  {
    uintptr_t tagged_ptr = io_uring_cqe_get_data64(cqe);

    struct voxy_chunk_database_wrapper *wrapper = tagged_ptr_value(tagged_ptr);
    switch(tagged_ptr_tag(tagged_ptr))
    {
    case TAG_OPEN_DIRECT:
      {
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
    case TAG_READV:
      {
        if(cqe->res == -ECANCELED)
          break;

        if(cqe->res < 0)
        {
          LOG_ERROR("Failed to read from file: %s: %s\n", wrapper->path, strerror(-cqe->res));
          break;
        }

        if(cqe->res != sizeof wrapper->chunk->block_ids + sizeof wrapper->chunk->block_light_levels)
        {
          LOG_ERROR("Partial read from file: %s: expected %zu bytes, got %d bytes", wrapper->path, sizeof wrapper->chunk->block_ids + sizeof wrapper->chunk->block_light_levels, cqe->res);
          break;
        }
      }
      break;
    case TAG_CLOSE_DIRECT:
      {
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
    }

    io_uring_cqe_seen(&chunk_database->ring, cqe);
  }
}
