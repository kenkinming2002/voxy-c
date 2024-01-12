#include <voxy/world.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

static float get_height(seed_t seed, int y, int x)
{
  struct vec2 position = vec2_zero();
  position = vec2_add(position, vec2(x, y));
  position = vec2_div(position, 17.0f);

  float value = perlin2(seed, position);
  value += 1.0f;
  value *= 0.5f;
  value *= 30.0f;
  return value;
}

void chunk_init(struct chunk *chunk, seed_t seed)
{
  if(chunk->z < 0)
    return; // Fast-path

  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_z = chunk->z * CHUNK_WIDTH + (int)z;
        int real_y = chunk->y * CHUNK_WIDTH + (int)y;
        int real_x = chunk->x * CHUNK_WIDTH + (int)x;
        if(real_z <= get_height(seed, real_y, real_x))
        {
          chunk->tiles[z][y][x].present = true;
          chunk->tiles[z][y][x].color.r = (float)rand() / (float)RAND_MAX;
          chunk->tiles[z][y][x].color.g = (float)rand() / (float)RAND_MAX;
          chunk->tiles[z][y][x].color.b = (float)rand() / (float)RAND_MAX;
        }
        else
          chunk->tiles[z][y][x].present = false;
      }

  chunk->remesh = true;
}

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

void world_deinit(struct world *world)
{
  free(world->chunks);
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

