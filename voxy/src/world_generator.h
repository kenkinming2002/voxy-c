#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "world.h"

struct section_info
{
  struct section_info *next;
  size_t               hash;

  int x, y;
  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
};

struct world_generator
{
  bool player_spawned;

  struct section_info **section_infos;
  size_t                section_info_capacity;
  size_t                section_info_load;
};

void world_generator_init(struct world_generator *world_generator);
void world_generator_deinit(struct world_generator *world_generator);

void world_generator_section_info_rehash(struct world_generator *world_generator, size_t new_capacity);
struct section_info *world_generator_section_info_insert(struct world_generator *world_generator, int x, int y);
struct section_info *world_generator_section_info_lookup(struct world_generator *world_generator, int x, int y);
struct section_info *world_generator_section_info_get(struct world_generator *world_generator, struct world *world, int x, int y);

void world_generator_update(struct world_generator *world_generator, struct world *world);
void world_generator_generate_chunk(struct world_generator *world_generator, struct world *world, int x, int y, int z);
float world_generator_get_height(struct world_generator *world_generator, struct world *world, int x, int y);

#endif // VOXY_WORLD_GENERATOR_H
