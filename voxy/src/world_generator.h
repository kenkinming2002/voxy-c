#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include <voxy/config.h>
#include <voxy/math/vector.h>

#include "resource_pack.h"
#include "config.h"
#include "thread_pool.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk_data_wrapper
#define SC_HASH_TABLE_NODE_TYPE struct chunk_data_wrapper
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

struct chunk_data_wrapper
{
  struct chunk_data_wrapper *next;
  struct chunk_data_wrapper *next_list;
  size_t                     hash;
  ivec3_t                    position;

  struct chunk_data * _Atomic chunk_data;
};

struct world_generator
{
  seed_t seed;

  struct thread_pool thread_pool;
  struct chunk_data_wrapper_hash_table chunk_data_wrappers;
};

void world_generator_init(struct world_generator *world_generator, seed_t seed);
void world_generator_fini(struct world_generator *world_generator);

/// On first call, the following functions may submit a job to a internal thread
/// pool in order to generate the relevant structures, and then return NULL to
/// indicate that the result is not yet available.
struct chunk_data *world_generator_generate_chunk_data(struct world_generator *world_generator, ivec3_t position, struct resource_pack *resource_pack);
fvec3_t            world_generator_generate_spawn     (struct world_generator *world_generator, struct resource_pack *resource_pack);

#endif // VOXY_WORLD_GENERATOR_H
