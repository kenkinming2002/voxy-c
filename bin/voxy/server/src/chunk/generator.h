#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include "future.h"

#include <voxy/server/chunk/generator.h>
#include <libcore/thread_pool.h>

struct voxy_chunk;
struct voxy_chunk_generator;

struct voxy_chunk_generator_wrapper
{
  struct voxy_chunk_generator_wrapper *next;
  size_t          hash;

  struct thread_pool_job job;

  ivec3_t position;
  struct voxy_chunk_generator *chunk_generator;
  struct voxy_context *context;
  struct voxy_chunk * _Atomic chunk;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX voxy_chunk_generator_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk_generator_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

struct voxy_chunk_generator
{
  seed_t seed;
  voxy_generate_chunk_t generate_chunk;
  struct voxy_chunk_generator_wrapper_hash_table wrappers;
};

void voxy_chunk_generator_init(struct voxy_chunk_generator *chunk_generator, seed_t seed);
void voxy_chunk_generator_fini(struct voxy_chunk_generator *chunk_generator);

struct chunk_future voxy_chunk_generator_generate(struct voxy_chunk_generator *chunk_generator, ivec3_t position, const struct voxy_context *context);

#endif // CHUNK_GENERATOR_H
