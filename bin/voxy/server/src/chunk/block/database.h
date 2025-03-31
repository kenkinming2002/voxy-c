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

struct voxy_block_database
{
  struct io_uring ring;
  size_t fixed_file_bitmaps[CHUNK_DATABASE_LOAD_LIMIT / SIZE_WIDTH];

  char *directory;
  struct block_database_load_entry *load_entries;
  struct block_database_save_entry *save_entries;
};

void voxy_block_database_init(struct voxy_block_database *block_database, const char *world_directory);
void voxy_block_database_fini(struct voxy_block_database *block_database);

struct block_group_future voxy_block_database_load(struct voxy_block_database *block_database, ivec3_t position);
struct unit_future voxy_block_database_save(struct voxy_block_database *block_database, ivec3_t position, struct voxy_block_group *block_group);

void voxy_block_database_update(struct voxy_block_database *block_database);

#endif // CHUNK_BLOCK_DATABASE_H
