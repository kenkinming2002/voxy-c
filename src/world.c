#include <voxy/world.h>

#include <stdlib.h>

void chunk_randomize(struct chunk *chunk)
{
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        chunk->tiles[z][y][x].present = rand() % 2 == 0;
        chunk->tiles[z][y][x].color.r = (float)rand() / (float)RAND_MAX;
        chunk->tiles[z][y][x].color.g = (float)rand() / (float)RAND_MAX;
        chunk->tiles[z][y][x].color.b = (float)rand() / (float)RAND_MAX;
      }

  chunk->remesh = true;
}

void world_init(struct world *world)
{
  world->chunks         = NULL;
  world->chunk_count    = 0;
  world->chunk_capacity = 0;
}

struct chunk *world_chunk_add(struct world *world, int z, int y, int x)
{
  if(world->chunk_count == world->chunk_capacity)
  {
    world->chunk_capacity = world->chunk_count != 0 ? world->chunk_count * 2 : 1;
    world->chunks         = realloc(world->chunks, world->chunk_capacity * sizeof world->chunks[0]);
  }
  struct chunk *chunk = &world->chunks[world->chunk_count++];
  chunk->z = z;
  chunk->y = y;
  chunk->x = x;
  return chunk;
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

