#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "world.h"

#define CAVE_WORM_RATIO 1e-3

#define CAVE_WORM_TRIAL 10
#define CAVE_WORM_NODE_COUNT  30
#define CAVE_WORM_NODE_RADIUS 5.0f
#define CAVE_WORM_STEP 5.0f

struct section_info
{
  struct section_info *next;
  size_t               hash;

  int x, y;
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

  int x, y, z;

  bool cave_generated;

  struct node *nodes;
  size_t       node_count;
  size_t       node_capacity;
};

struct world_generator
{
  bool player_spawned;

  struct section_info **section_infos;
  size_t                section_info_capacity;
  size_t                section_info_load;

  struct chunk_info **chunk_infos;
  size_t              chunk_info_capacity;
  size_t              chunk_info_load;
};

void world_generator_init(struct world_generator *world_generator);
void world_generator_deinit(struct world_generator *world_generator);

void world_generator_section_info_rehash(struct world_generator *world_generator, size_t new_capacity);
struct section_info *world_generator_section_info_insert(struct world_generator *world_generator, int x, int y);
struct section_info *world_generator_section_info_lookup(struct world_generator *world_generator, int x, int y);
struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, int x, int y);

void world_generator_chunk_info_rehash(struct world_generator *world_generator, size_t new_capacity);
struct chunk_info *world_generator_chunk_info_insert(struct world_generator *world_generator, int x, int y, int z);
struct chunk_info *world_generator_chunk_info_lookup(struct world_generator *world_generator, int x, int y, int z);
struct chunk_info *world_generator_chunk_info_lookup_or_insert(struct world_generator *world_generator, int x, int y, int z);

struct chunk_info *world_generator_chunk_info_get(struct world_generator *world_generator, struct world *world, int x, int y, int z);

void world_generator_generate_cave(struct world_generator *world_generator, struct world *world, int x, int y, int z);
void world_generator_add_node(struct world_generator *world_generator, struct vec3 position);

void world_generator_update(struct world_generator *world_generator, struct world *world);
void world_generator_update_at(struct world_generator *world_generator, struct world *world, int x, int y, int z);

#endif // VOXY_WORLD_GENERATOR_H
