#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "config.h"
#include "thread_pool.h"
#include "vector.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX section_info
#define SC_HASH_TABLE_NODE_TYPE struct section_info
#define SC_HASH_TABLE_KEY_TYPE ivec2_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk_info
#define SC_HASH_TABLE_NODE_TYPE struct chunk_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>

struct section_info
{
  struct section_info *next;
  struct section_info *next_list;
  size_t               hash;
  ivec2_t         position;

  atomic_bool done;
  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct chunk_info
{
  struct chunk_info *next;
  struct chunk_info *next_list;
  size_t             hash;
  ivec3_t       position;

  atomic_bool done;
  bool caves[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]; // FIXME: bitmap?
};

struct world_generator
{
  seed_t seed;

  struct thread_pool thread_pool;
  struct section_info_hash_table section_infos;
  struct chunk_info_hash_table   chunk_infos;
};

void world_generator_init(struct world_generator *world_generator, seed_t seed);
void world_generator_fini(struct world_generator *world_generator);

/// On first call, the following functions may submit a job to a internal thread
/// pool in order to generate the relevant structures, and then return NULL to
/// indicate that the result is not yet available.
struct section_info *world_generator_generate_section_info(struct world_generator *world_generator, ivec2_t position);
struct chunk_info *world_generator_generate_chunk_info(struct world_generator *world_generator, ivec3_t position);
struct chunk_data *world_generator_generate_chunk_data(struct world_generator *world_generator, ivec3_t position);

float world_generator_get_height(struct world_generator *world_generator, ivec2_t position);

#endif // VOXY_WORLD_GENERATOR_H
