#include "generator.h"

#include "chunk.h"

#include <libcommon/math/noise.h>

#include <string.h>

void chunk_generator_init(struct chunk_generator *chunk_generator, seed_t seed)
{
  chunk_generator->seed = seed;
}

void chunk_generator_fini(struct chunk_generator *chunk_generator)
{
  (void)chunk_generator;
}

struct chunk *chunk_generator_generate(struct chunk_generator *chunk_generator, ivec3_t position)
{
  struct chunk *chunk = chunk_create();
  chunk->position = position;

  // Generate height map
  float heights[VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH];
  for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
    for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
    {
      const ivec2_t position = ivec2_add(ivec2_mul_scalar(ivec2(chunk->position.x, chunk->position.y), VOXY_CHUNK_WIDTH), ivec2(x, y));
      heights[y][x] = noise_perlin2_ex(chunk_generator->seed, ivec2_as_fvec2(position), 0.008f, 2.0f, 0.3f, 8) * 50.0f;
    }

  // Populate blocks
  for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const int height = VOXY_CHUNK_WIDTH * chunk->position.z + z;
        if(height > heights[y][x])
          chunk->block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = 0;
        else if(height > heights[y][x] - 1)
          chunk->block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = 3;
        else
          chunk->block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = 2;
      }

  // Set light levels.
  //
  // FIXME: Actual light system.
  memset(&chunk->block_light_levels, 0xFF, sizeof chunk->block_light_levels);

  return chunk;
}
