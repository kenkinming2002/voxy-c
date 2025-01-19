#include "database.h"

#include "chunk.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcore/log.h>
#include <libcore/fs.h>
#include <libcore/tformat.h>

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
  close(wrapper->aiocb.aio_fildes);
  free(wrapper);
}

#define chunk_dir(position) tformat("world/chunks/%d/%d/%d", position.x, position.y, position.z)
#define chunk_file(position) tformat("world/chunks/%d/%d/%d/blocks", position.x, position.y, position.z)

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database)
{
  voxy_chunk_database_wrapper_hash_table_init(&chunk_database->wrappers);
}

void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database)
{
  // We have to cancel all outgoing aio request. Unfortunately,
  // aio_cancel() may not be supported, in which case we have no choice but to
  // keep the aiocb structure alive. This means that we cannot simply dispose of
  // all wrappers unconditionally here. In theory, we could check if all
  // cancellations are successful but that does not really matter, since we
  // would be shutting down and the the OS will clean up everything anyway.
  //
  // FIXME: Consider if we need to change it.
  struct voxy_chunk_database_wrapper *wrapper;
  SC_HASH_TABLE_FOREACH(chunk_database->wrappers, wrapper)
    aio_cancel(wrapper->aiocb.aio_fildes, &wrapper->aiocb);
}

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position)
{
  // In the following code, we may return chunk_future_pending in case of an
  // error. This is a hack since we cannot be sure if the error is transient.
  // For example, we may failed to open a file due to permission problem but the
  // file actually exist. In such a case, we want to prevent the caller from
  // trying to generate the chunk instead, thinking that it has not yet been
  // generated. This is accomplished by lying to the caller that we are in the
  // process of loading the chunk, even though it will never actually complete
  // because we are not actually doing anything.

  const char *file = chunk_file(position);

  struct voxy_chunk_database_wrapper *wrapper = voxy_chunk_database_wrapper_hash_table_lookup(&chunk_database->wrappers, position);
  if(!wrapper)
  {
    if(chunk_database->wrappers.load >= CHUNK_DATABASE_LOAD_LIMIT)
      return chunk_future_pending;

    int fd = open(file, O_RDONLY);
    if(fd == -1)
    {
      if(errno == ENOENT)
        return chunk_future_reject;

      LOG_ERROR("Failed to open file %s: %s", file, strerror(errno));
      return chunk_future_pending;
    }

    wrapper = malloc(sizeof *wrapper);
    wrapper->position = position;

    memset(&wrapper->aiocb, 0, sizeof wrapper->aiocb);
    wrapper->aiocb.aio_fildes = fd;
    wrapper->aiocb.aio_buf = wrapper->buf;
    wrapper->aiocb.aio_nbytes = sizeof wrapper->buf;
    aio_read(&wrapper->aiocb);

    voxy_chunk_database_wrapper_hash_table_insert(&chunk_database->wrappers, wrapper);
  }

resume:
  if(aio_error(&wrapper->aiocb) == EINPROGRESS)
    return chunk_future_pending;

  ssize_t result = aio_return(&wrapper->aiocb);
  if(result == -1)
  {
    LOG_ERROR("Failed to read from file %s: %s", file, strerror(errno));
    return chunk_future_pending;
  }

  wrapper->aiocb.aio_buf += result;
  wrapper->aiocb.aio_nbytes -= result;
  if(wrapper->aiocb.aio_nbytes != 0)
  {
    aio_read(&wrapper->aiocb);
    goto resume;
  }

  struct voxy_chunk *chunk = voxy_chunk_create();
  chunk->position = position;
  chunk->disk_dirty = false;
  chunk->network_dirty = true;

  libserde_deserializer_t deserializer = libserde_deserializer_create_mem(wrapper->buf, sizeof wrapper->buf);
  if(!deserializer)
    goto err_chunk_destroy;

  libserde_deserializer_try_read(deserializer, chunk->block_ids, err_deserializer_destroy);
  libserde_deserializer_try_read(deserializer, chunk->block_light_levels, err_deserializer_destroy);
  libserde_deserializer_try_read_checksum(deserializer, err_deserializer_destroy);
  libserde_deserializer_destroy(deserializer);

  voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wrappers, position);
  voxy_chunk_database_wrapper_dispose(wrapper);
  return chunk_future_ready(chunk);

err_deserializer_destroy:
  libserde_deserializer_destroy(deserializer);
err_chunk_destroy:
  voxy_chunk_destroy(chunk);
  voxy_chunk_database_wrapper_hash_table_remove(&chunk_database->wrappers, position);
  voxy_chunk_database_wrapper_dispose(wrapper);
  return chunk_future_pending;
}

int voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk)
{
  const char *dir = chunk_dir(chunk->position);
  if(mkdir_recursive(dir) != 0)
    goto err;

  const char *file = chunk_file(chunk->position);

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

