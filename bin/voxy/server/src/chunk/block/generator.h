#ifndef CHUNK_BLOCK_GENERATOR_H
#define CHUNK_BLOCK_GENERATOR_H

#include "future.h"

#include <voxy/server/chunk/block/generator.h>
#include <libcore/thread_pool.h>

struct voxy_block_group;
struct voxy_block_generator;

struct voxy_block_generator_wrapper
{
  struct voxy_block_generator_wrapper *next;
  size_t          hash;

  struct thread_pool_job job;

  ivec3_t position;
  struct voxy_block_generator *block_generator;
  struct voxy_context *context;
  struct voxy_block_group * _Atomic block_group;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX voxy_block_generator_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_block_generator_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

struct voxy_block_generator
{
  seed_t seed;
  struct voxy_block_generator_wrapper_hash_table wrappers;

  voxy_generate_block_t generate_block;
};

void voxy_block_generator_init(struct voxy_block_generator *block_generator, const char *world_directory);
void voxy_block_generator_fini(struct voxy_block_generator *block_generator);

struct block_group_future voxy_block_generator_generate(struct voxy_block_generator *block_generator, ivec3_t position, const struct voxy_context *context);

#endif // CHUNK_BLOCK_GENERATOR_H
