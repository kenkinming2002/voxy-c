#ifndef VOXY_WORLD_GENERATOR_H
#define VOXY_WORLD_GENERATOR_H

#include "world.h"

struct world_generator
{
  bool player_spawned;
};

void world_generator_init(struct world_generator *world_generator);
void world_generator_deinit(struct world_generator *world_generator);

void world_generator_update(struct world_generator *world_generator, struct world *world);
void world_generator_generate_chunk(struct world_generator *world_generator, struct world *world, int x, int y, int z);
float world_generator_get_height(struct world_generator *world_generator, struct world *world, int x, int y);

#endif // VOXY_WORLD_GENERATOR_H
