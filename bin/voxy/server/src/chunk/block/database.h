#ifndef CHUNK_BLOCK_DATABASE_H
#define CHUNK_BLOCK_DATABASE_H

#include "future.h"
#include "group.h"

#include <voxy/config.h>

#include <libmath/vector.h>

#include <liburing.h>
#include <aio.h>

#define CHUNK_DATABASE_LOAD_LIMIT 512

struct voxy_block_group;

struct voxy_block_database_load_wrapper
{
  struct voxy_block_database_load_wrapper *next;
  size_t hash;

  ivec3_t position;

  int fixed_file;

  char *path;
  struct voxy_block_group *block_group;
  struct iovec iovecs[2];

  bool done;
};

struct voxy_block_database_save_wrapper
{
  struct voxy_block_database_save_wrapper *next;
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

struct voxy_block_database
{
  struct io_uring ring;
  size_t fixed_file_bitmaps[CHUNK_DATABASE_LOAD_LIMIT / SIZE_WIDTH];

  char *directory;
  struct voxy_block_database_load_wrapper_hash_table load_wrappers;
  struct voxy_block_database_save_wrapper_hash_table save_wrappers;
};

void voxy_block_database_init(struct voxy_block_database *block_database, const char *world_directory);
void voxy_block_database_fini(struct voxy_block_database *block_database);

struct block_group_future voxy_block_database_load(struct voxy_block_database *block_database, ivec3_t position);
struct unit_future voxy_block_database_save(struct voxy_block_database *block_database, ivec3_t position, struct voxy_block_group *block_group);

void voxy_block_database_update(struct voxy_block_database *block_database);

#endif // CHUNK_BLOCK_DATABASE_H
