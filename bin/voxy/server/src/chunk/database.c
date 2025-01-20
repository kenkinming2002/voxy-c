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
  LIBCORE_UNREACHABLE;
}

#define chunk_dir(position) "world/chunks/%d/%d/%d", position.x, position.y, position.z
#define chunk_file(position) "world/chunks/%d/%d/%d/blocks", position.x, position.y, position.z

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database)
{
  io_uring_queue_init(32, &chunk_database->ring, 0);
  voxy_chunk_database_wrapper_hash_table_init(&chunk_database->done);
  voxy_chunk_database_wrapper_hash_table_init(&chunk_database->wait_sqe);
  voxy_chunk_database_wrapper_hash_table_init(&chunk_database->wait_cqe);
}

void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database)
{
  io_uring_queue_exit(&chunk_database->ring);

  // The problem is that there may be request to open/close file descriptor
  // in-flight before we exit io uring. Hence, file descriptors in our hash
  // table can be in a schrodinger's cat situation. We do not know if they are
  // valid or not and we cannot close them because the same file descriptor may
  // have been reused.
  //
  // We could absolutely try to engineer a perfect solution to this problem, but
  // leaking some memory and file descriptors during shutdown is not the end of
  // the world.
}

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position)
{
  profile_scope;

  struct voxy_chunk_database_wrapper *wrapper;
  if((wrapper = voxy_chunk_database_wrapper_hash_table_lookup(&chunk_database->done, position)))
  {
    voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->done, position);

    struct voxy_chunk *chunk = wrapper->chunk;
    wrapper->chunk = NULL;
    if(!chunk)
      return chunk_future_reject;
    return chunk_future_ready(chunk);
  }

  if(voxy_chunk_database_wrapper_hash_table_lookup(&chunk_database->wait_sqe, position)) return chunk_future_pending;
  if(voxy_chunk_database_wrapper_hash_table_lookup(&chunk_database->wait_cqe, position)) return chunk_future_pending;

  if(chunk_database->wait_sqe.load + chunk_database->wait_cqe.load >= CHUNK_DATABASE_LOAD_LIMIT)
    return chunk_future_pending;

  wrapper = malloc(sizeof *wrapper);
  wrapper->position = position;
  wrapper->state = VOXY_CHUNK_DATABASE_WRAPPER_STATE_OPEN;
  wrapper->path = aformat(chunk_file(position));
  wrapper->fd = -1;
  wrapper->chunk = NULL;
  voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wait_sqe, wrapper);

  return chunk_future_pending;
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

static void voxy_chunk_database_update_sqe(struct voxy_chunk_database *chunk_database)
{
  profile_scope;

  // Our hash table is simply an array of singly-linked list. What we do is to
  // loop through each singly-linked list repeatedly dequeue the first item.
  for(size_t i=0; i<chunk_database->wait_sqe.bucket_count; ++i)
    while(chunk_database->wait_sqe.buckets[i].head)
    {
      struct io_uring_sqe *sqe = io_uring_get_sqe(&chunk_database->ring);
      if(!sqe)
        goto done;

      struct voxy_chunk_database_wrapper *wrapper = chunk_database->wait_sqe.buckets[i].head;
      chunk_database->wait_sqe.buckets[i].head = chunk_database->wait_sqe.buckets[i].head->next;
      chunk_database->wait_sqe.load -= 1;
      switch(wrapper->state)
      {
      case VOXY_CHUNK_DATABASE_WRAPPER_STATE_OPEN:
        io_uring_prep_open(sqe, wrapper->path, O_RDONLY, 0);
        break;
      case VOXY_CHUNK_DATABASE_WRAPPER_STATE_READ:
        io_uring_prep_readv(sqe, wrapper->fd, wrapper->iovecs, sizeof wrapper->iovecs / sizeof wrapper->iovecs[0], 0);
        break;
      case VOXY_CHUNK_DATABASE_WRAPPER_STATE_CLOSE:
        io_uring_prep_close(sqe, wrapper->fd);
        break;
      }
      io_uring_sqe_set_data(sqe, wrapper);
      voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wait_cqe, wrapper);
    }

done:
  io_uring_submit(&chunk_database->ring);
}

static void voxy_chunk_database_handle_cqe(struct voxy_chunk_database *chunk_database, struct io_uring_cqe *cqe)
{
  struct voxy_chunk_database_wrapper *wrapper = io_uring_cqe_get_data(cqe);
  switch(wrapper->state)
  {
  case VOXY_CHUNK_DATABASE_WRAPPER_STATE_OPEN:
    {
      free(wrapper->path);

      int fd = cqe->res;

      if(fd == -ENOENT)
      {
        voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wait_cqe, wrapper->position);
        voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->done, wrapper);
        break;
      }

      if(fd < 0)
      {
        LOG_ERROR("Failed to open file: %s: %s", wrapper->path, strerror(-fd));
        break;
      }

      wrapper->state = VOXY_CHUNK_DATABASE_WRAPPER_STATE_READ;

      wrapper->fd = fd;

      wrapper->chunk = voxy_chunk_create();
      wrapper->chunk->position = wrapper->position;
      wrapper->chunk->disk_dirty = false;
      wrapper->chunk->network_dirty = true;

      wrapper->iovecs[0].iov_base = wrapper->chunk->block_ids;
      wrapper->iovecs[0].iov_len = sizeof wrapper->chunk->block_ids;
      wrapper->iovecs[1].iov_base = wrapper->chunk->block_light_levels;
      wrapper->iovecs[1].iov_len = sizeof wrapper->chunk->block_light_levels;

      voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wait_cqe, wrapper->position);
      voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wait_sqe, wrapper);
    }
    break;
  case VOXY_CHUNK_DATABASE_WRAPPER_STATE_READ:
    {
      ssize_t n = cqe->res;
      if(n < 0)
      {
        LOG_ERROR("Failed to read from file: %s: %s", wrapper->path, strerror(-n));
        break;
      }

      // We are reading from regular files. This should never happen - famous
      // last word. See
      // https://stackoverflow.com/questions/37042287/read-from-regular-file-block-or-return-less-data
      // for an hypothetical scenario in which this might happen. Since
      // further read should return error anyway, we may as well fail early.
      if(n != sizeof wrapper->chunk->block_ids + sizeof wrapper->chunk->block_light_levels)
      {
        LOG_ERROR("Partial read from file: %s: expected %zu bytes, got %zu bytes", wrapper->path, sizeof wrapper->chunk->block_ids + sizeof wrapper->chunk->block_light_levels, n);
        break;
      }

      wrapper->state = VOXY_CHUNK_DATABASE_WRAPPER_STATE_CLOSE;

      voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wait_cqe, wrapper->position);
      voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wait_sqe, wrapper);
    }
    break;
  case VOXY_CHUNK_DATABASE_WRAPPER_STATE_CLOSE:
    {
      int result = cqe->res;
      if(result < 0)
        LOG_ERROR("Failed to close file: %s: %s", wrapper->path, strerror(-result));

      wrapper->fd = -1;

      voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wait_cqe, wrapper->position);
      voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->done, wrapper);
    }
    break;
  }
}

static void voxy_chunk_database_update_cqe(struct voxy_chunk_database *chunk_database)
{
  profile_scope;

  struct io_uring_cqe *cqe;
  while(io_uring_peek_cqe(&chunk_database->ring, &cqe) == 0)
  {
    voxy_chunk_database_handle_cqe(chunk_database, cqe);
    io_uring_cqe_seen(&chunk_database->ring, cqe);
  }
}

void voxy_chunk_database_update(struct voxy_chunk_database *chunk_database)
{
  profile_scope;

  voxy_chunk_database_update_sqe(chunk_database);
  voxy_chunk_database_update_cqe(chunk_database);
}
