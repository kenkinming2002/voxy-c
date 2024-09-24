#include "generator.h"

#include "chunk.h"

#include <libcommon/math/noise.h>

void chunk_generator_init(struct chunk_generator *chunk_generator, seed_t seed)
{
  chunk_generator->seed = seed;
}

void chunk_generator_fini(struct chunk_generator *chunk_generator)
{
  (void)chunk_generator;
}

struct chunk *chunk_generator_generate(struct chunk_generator *chunk_generator, ivec3_t position, struct voxy_block_registry *block_registry)
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

  // Populate block ids and light levels.
  for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const int height = VOXY_CHUNK_WIDTH * chunk->position.z + z;
        const uint8_t id
          = height > 64 ? 1
          : height > heights[y][x] ? 0
          : height > heights[y][x] - 1 ? 3
          : 2;

        const uint8_t light_level = voxy_block_registry_query_block(block_registry, id).light_level;

        chunk_set_block_id(chunk, ivec3(x, y, z), id);
        chunk_set_block_light_level(chunk, ivec3(x, y, z), light_level);
      }

  chunk->disk_dirty = true;
  chunk->network_dirty = true;

  return chunk;
}
