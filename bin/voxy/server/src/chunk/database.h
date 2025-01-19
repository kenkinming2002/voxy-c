#ifndef CHUNK_DATABASE_H
#define CHUNK_DATABASE_H

#include "future.h"
#include "chunk.h"

#include <voxy/config.h>

#include <libmath/vector.h>

#include <aio.h>

#define member_sizeof(type, member) sizeof ((type *)0)->member

struct voxy_chunk;

struct voxy_chunk_database_wrapper
{
  struct voxy_chunk_database_wrapper *next;
  size_t hash;

  ivec3_t position;

  struct aiocb aiocb;
  char buf[member_sizeof(struct voxy_chunk, block_ids)
         + member_sizeof(struct voxy_chunk, block_light_levels)
         + 1];
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX voxy_chunk_database_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_database_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

struct voxy_chunk_database
{
  struct voxy_chunk_database_wrapper_hash_table wrappers;
};

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database);
void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database);

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position);
int voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk);

#endif // CHUNK_DATABASE_H
