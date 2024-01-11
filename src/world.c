#include <voxy/world.h>

#include <stdlib.h>

void world_init(struct world *world)
{
  world->chunks         = NULL;
  world->chunk_count    = 0;
  world->chunk_capacity = 0;
}

void world_chunk_add(struct world *world, struct chunk chunk)
{
  if(world->chunk_count == world->chunk_capacity)
  {
    world->chunk_capacity = world->chunk_count != 0 ? world->chunk_count * 2 : 1;
    world->chunks         = realloc(world->chunks, world->chunk_capacity * sizeof world->chunks[0]);
  }
  world->chunks[world->chunk_count++] = chunk;
}

struct chunk *world_chunk_lookup(struct world *world, int z, int y, int x)
{
  for(size_t i=0; i<world->chunk_count; ++i)
  {
    if(world->chunks[i].z != z) continue;
    if(world->chunks[i].y != y) continue;
    if(world->chunks[i].x != x) continue;
    return &world->chunks[i];
  }
  return NULL;
}

