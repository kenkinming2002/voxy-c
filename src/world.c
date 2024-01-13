#include <voxy/world.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>

static float get_height(seed_t seed, int y, int x)
{
  float value = 0.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 15.0f)) * 30.0f + 30.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 7.0f))  * 15.0f + 10.0f;
  value += perlin2(seed, vec2_div_s(vec2(x, y), 3.0f))  * 3.0f  + 3.0f;
  return value;
}

void chunk_init(struct chunk *chunk, seed_t seed)
{
  if(chunk->z < 0)
    return; // Fast-path

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_y = chunk->y * CHUNK_WIDTH + (int)y;
      int real_x = chunk->x * CHUNK_WIDTH + (int)x;
      heights[y][x] = get_height(seed, real_y, real_x);
    }

  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_z = chunk->z * CHUNK_WIDTH + (int)z;
        if(real_z <= heights[y][x])
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

