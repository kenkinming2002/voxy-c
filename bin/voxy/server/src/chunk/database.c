#include "database.h"

#include <libserde/serializer.h>
#include <libserde/deserializer.h>

#include <libcommon/core/fs.h>

#include <stdio.h>
#include <stdlib.h>

static char *get_chunk_dirpath(ivec3_t position)
{
  char *s;
  asprintf(&s, "world/chunks/%d/%d/%d", position.x, position.y, position.z);
  return s;
}

static char *get_chunk_filepath(ivec3_t position)
{
  char *s;
  asprintf(&s, "world/chunks/%d/%d/%d/blocks", position.x, position.y, position.z);
  return s;
}

struct voxy_chunk *voxy_chunk_database_load(ivec3_t position)
{
  char *dirpath = get_chunk_dirpath(position);
  if(!dirpath)
    goto error1;

  if(mkdir_recursive(dirpath) != 0)
    goto error1;

  char *filepath = get_chunk_filepath(position);
  if(!filepath)
    goto error2;

  libserde_deserializer_t deserializer = libserde_deserializer_create(filepath);
  if(!deserializer)
    goto error3;

  struct voxy_chunk *chunk = voxy_chunk_create();
  if(!chunk)
    goto error4;

  chunk->position = position;

  libserde_deserializer_try_read(deserializer, chunk->block_ids, error5);
  libserde_deserializer_try_read(deserializer, chunk->block_light_levels, error5);
  libserde_deserializer_try_read_checksum(deserializer, error5);

  chunk->disk_dirty = false;
  chunk->network_dirty = true;

  libserde_deserializer_destroy(deserializer);
  free(dirpath);
  free(filepath);
  return chunk;

error5:
  libserde_deserializer_destroy(deserializer);
error4:
  voxy_chunk_destroy(chunk);
error3:
  free(filepath);
error2:
  free(dirpath);
error1:
  return NULL;
}

int voxy_chunk_database_save(struct voxy_chunk *chunk)
{
  char *filepath = get_chunk_filepath(chunk->position);
  if(!filepath)
    goto error1;

  libserde_serializer_t serializer = libserde_serializer_create(filepath);
  if(!serializer)
    goto error2;

  libserde_serializer_try_write(serializer, chunk->block_ids, error3);
  libserde_serializer_try_write(serializer, chunk->block_light_levels, error3);
  libserde_serializer_try_write_checksum(serializer, error3);

  libserde_serializer_destroy(serializer);
  free(filepath);
  return 0;

error3:
  libserde_serializer_destroy(serializer);
error2:
  free(filepath);
error1:
  return -1;
}

