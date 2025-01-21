#ifndef CHUNK_DATABASE_H
#define CHUNK_DATABASE_H

#include "future.h"
#include "chunk.h"

#include <voxy/config.h>

#include <libmath/vector.h>

#include <liburing.h>
#include <aio.h>

#define CHUNK_DATABASE_LOAD_LIMIT 512

struct voxy_chunk;

struct voxy_chunk_database_load_wrapper
{
  struct voxy_chunk_database_load_wrapper *next;
  size_t hash;

  ivec3_t position;

  int fixed_file;

  char *path;
  struct voxy_chunk *chunk;
  struct iovec iovecs[2];

  bool done;
};

struct voxy_chunk_database_save_wrapper
{
  struct voxy_chunk_database_save_wrapper *next;
  size_t hash;

  ivec3_t position;

  int fixed_file;

  char *dirs[3];
  char *path;
  struct iovec iovecs[2];

  bool done;
};

#define SC_HASH_TABLE_INTERFACE
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

struct voxy_chunk_database
{
  struct io_uring ring;
  size_t fixed_file_bitmaps[CHUNK_DATABASE_LOAD_LIMIT / SIZE_WIDTH];
  struct voxy_chunk_database_load_wrapper_hash_table load_wrappers;
  struct voxy_chunk_database_save_wrapper_hash_table save_wrappers;
};

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database);
void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database);

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position);
struct unit_future voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk);

void voxy_chunk_database_update(struct voxy_chunk_database *chunk_database);

#endif // CHUNK_DATABASE_H
