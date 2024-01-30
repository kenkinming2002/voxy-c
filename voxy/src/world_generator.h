#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "world.h"
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

#include <stdatomic.h>
#include <pthread.h>

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

  atomic_bool thread_shutdown;
  int         thread_count;

  struct section_info_hash_table  section_infos;
  struct section_info            *section_infos_pending;
  pthread_mutex_t                 section_infos_mutex;
  pthread_cond_t                  section_infos_cond;
  pthread_t                      *section_infos_threads;

  struct chunk_info_hash_table  chunk_infos;
  struct chunk_info            *chunk_infos_pending;
  pthread_mutex_t               chunk_infos_mutex;
  pthread_cond_t                chunk_infos_cond;
  pthread_t                    *chunk_infos_threads;
};

void world_generator_init(struct world_generator *world_generator, seed_t seed);
void world_generator_deinit(struct world_generator *world_generator);

void world_generator_update(struct world_generator *world_generator, struct world *world);
void world_generator_update_spawn_player(struct world_generator *world_generator, struct world *world);
void world_generator_update_generate_chunks(struct world_generator *world_generator, struct world *world);
void world_generator_update_generate_chunk(struct world_generator *world_generator, struct world *world, ivec3_t chunk_position);

#endif // VOXY_WORLD_GENERATOR_H
