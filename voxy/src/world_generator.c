#include "world_generator.h"

void world_generator_init(struct world_generator *world_generator)
{
  world_generator->player_spawned = false;
}

void world_generator_deinit(struct world_generator *world_generator)
{
  (void)world_generator;
}

void world_generator_update(struct world_generator *world_generator, struct world *world)
{
  if(!world_generator->player_spawned)
  {
    world_generator->player_spawned     = true;
    world->player_transform.translation = vec3(0.5f, 0.5f, world_generator_get_height(world_generator, world, 0, 0) + 2.0f);
    world->player_transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  }

  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);
  for(int dz = -8; dz<=8; ++dz)
    for(int dy = -8; dy<=8; ++dy)
      for(int dx = -8; dx<=8; ++dx)
        if(!world_chunk_lookup(world, x+dx, y+dy, z+dz))
          world_generator_generate_chunk(world_generator, world, x+dx, y+dy, z+dz);
}

void world_generator_generate_chunk(struct world_generator *world_generator, struct world *world, int x, int y, int z)
{
  (void)world_generator;

  struct chunk *chunk = world_chunk_add(world, x, y, z);
  chunk->mesh_dirty = false;

  if(chunk->z < 0)
  {
    for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
      for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
        for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;

    return; // Fast-path!?
  }

  float heights[CHUNK_WIDTH][CHUNK_WIDTH];
  for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
    for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
    {
      int real_x = chunk->x * CHUNK_WIDTH + (int)x;
      int real_y = chunk->y * CHUNK_WIDTH + (int)y;
      heights[y][x] = world_generator_get_height(world_generator, world, real_x, real_y);
    }

  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
      {
        int real_z = chunk->z * CHUNK_WIDTH + (int)z;
        if(real_z <= heights[y][x])
          chunk->tiles[z][y][x].id = TILE_ID_STONE;
        else if(real_z <= heights[y][x] + 1.0f)
          chunk->tiles[z][y][x].id = TILE_ID_GRASS;
        else
          chunk->tiles[z][y][x].id = TILE_ID_EMPTY;
      }

  struct chunk_adjacency chunk_adjacency;
  chunk_adjacency_init(&chunk_adjacency, world, chunk);
  if(chunk_adjacency.chunk)  chunk_adjacency.chunk ->mesh_dirty = true;
  if(chunk_adjacency.bottom) chunk_adjacency.bottom->mesh_dirty = true;
  if(chunk_adjacency.top)    chunk_adjacency.top   ->mesh_dirty = true;
  if(chunk_adjacency.back)   chunk_adjacency.back  ->mesh_dirty = true;
  if(chunk_adjacency.front)  chunk_adjacency.front ->mesh_dirty = true;
  if(chunk_adjacency.left)   chunk_adjacency.left  ->mesh_dirty = true;
  if(chunk_adjacency.right)  chunk_adjacency.right ->mesh_dirty = true;
}

float world_generator_get_height(struct world_generator *world_generator, struct world *world, int x, int y)
{
  (void)world_generator;

  float value = 0.0f;

  seed_t seed = world->seed;

  // Truly Massive Mountains
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 8000.0f)) * 2048.0f + 2048.0f;
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 4000.0f)) * 1024.0f + 1024.0f;

  // Mountains
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 1600.0f)) * 512.0f + 512.0f;
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 800.0f))  * 256.0f + 256.0f;

  // Hills
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 400.0f)) * 256.0f + 256.0f;
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 200.0f)) * 128.0f + 128.0f;

  // Small hills
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 10.0f)) * 5.0f + 5.0f;
  seed_next(&seed); value += perlin2(seed, vec2_div_s(vec2(x, y), 5.0f))  * 2.0f + 2.0f;

  return value;
}
