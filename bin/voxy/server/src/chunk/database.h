#ifndef CHUNK_DATABASE_H
#define CHUNK_DATABASE_H

#include "future.h"
#include "chunk.h"

#include <voxy/config.h>

#include <libmath/vector.h>

#include <liburing.h>
#include <aio.h>

/// NOTE: I am aware that the name of some of the types are unwieldily long. The
/// alternative is being able to put those types inside the source file so that
/// I do not have to worry about name collision. But C being C means that it is
/// not possible.

#define member_sizeof(type, member) sizeof ((type *)0)->member

struct voxy_chunk;

enum voxy_chunk_database_wrapper_state
{
  VOXY_CHUNK_DATABASE_WRAPPER_STATE_OPEN,
  VOXY_CHUNK_DATABASE_WRAPPER_STATE_READ,
  VOXY_CHUNK_DATABASE_WRAPPER_STATE_CLOSE,
};

struct voxy_chunk_database_wrapper
{
  struct voxy_chunk_database_wrapper *next;
  size_t hash;

  ivec3_t position;

  enum voxy_chunk_database_wrapper_state state;

  char *path;
  int fd;
  struct voxy_chunk *chunk;

  struct iovec iovecs[2];
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
  struct io_uring ring;

  struct voxy_chunk_database_wrapper_hash_table done;
  struct voxy_chunk_database_wrapper_hash_table wait_sqe;
  struct voxy_chunk_database_wrapper_hash_table wait_cqe;
};

void voxy_chunk_database_init(struct voxy_chunk_database *chunk_database);
void voxy_chunk_database_fini(struct voxy_chunk_database *chunk_database);

struct chunk_future voxy_chunk_database_load(struct voxy_chunk_database *chunk_database, ivec3_t position);
int voxy_chunk_database_save(struct voxy_chunk_database *chunk_database, struct voxy_chunk *chunk);

void voxy_chunk_database_update(struct voxy_chunk_database *chunk_database);

#endif // CHUNK_DATABASE_H
