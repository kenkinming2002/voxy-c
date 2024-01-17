#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "world.h"
#include "lin.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX section_info
#define SC_HASH_TABLE_NODE_TYPE struct section_info
#define SC_HASH_TABLE_KEY_TYPE struct ivec2
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk_info
#define SC_HASH_TABLE_NODE_TYPE struct chunk_info
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#define CAVE_WORM_RATIO 1e-3

#define CAVE_WORM_TRIAL 10
#define CAVE_WORM_NODE_COUNT  30
#define CAVE_WORM_NODE_RADIUS 5.0f
#define CAVE_WORM_STEP 5.0f

struct section_info
{
  struct section_info *next;
  size_t               hash;

  struct ivec2 position;

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct node
{
  struct vec3 position;
};

struct chunk_info
{
  struct chunk_info *next;
  size_t             hash;

  struct ivec3 position;

  bool cave_generated;

  struct node *nodes;
  size_t       node_count;
  size_t       node_capacity;
};

struct world_generator
{
  bool player_spawned;

  struct section_info_hash_table section_infos;
  struct chunk_info_hash_table   chunk_infos;
};

void world_generator_init(struct world_generator *world_generator);
void world_generator_deinit(struct world_generator *world_generator);

void world_generator_update(struct world_generator *world_generator, struct world *world);
void world_generator_update_spawn_player(struct world_generator *world_generator, struct world *world);
void world_generator_update_generate_chunks(struct world_generator *world_generator, struct world *world);
void world_generator_update_generate_chunk(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position);

struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, struct ivec2 section_position);
struct chunk_info *world_generator_chunk_info_get(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position);

void world_generator_generate_cave(struct world_generator *world_generator, struct world *world, struct ivec3 chunk_position);
void world_generator_add_node(struct world_generator *world_generator, struct vec3 position);

#endif // VOXY_WORLD_GENERATOR_H
