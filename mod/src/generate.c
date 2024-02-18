#include "ids.h"

#include <voxy/main_game/config.h>
#include <voxy/math/noise.h>
#include <voxy/math/random.h>

#include <stdbool.h>

#define ETHER_HEIGHT 128
#define WATER_HEIGHT 3

static inline float lerpf(float a, float b, float t)
{
  return a + (b - a) * t;
}

static inline float get_height(seed_t seed, ivec2_t position)
{
  float value1 = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 1/200.0f, 2.3f, 0.4f, 8);
  value1 = fabs(value1);
  value1 = powf(value1, 2.0f);
  value1 = ease_out(value1, 3.0f);
  value1 = value1 * 100.0f;

  float value2 = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 1/2000.0f, 2.1f, 0.6f, 8);
  value2 = 0.5f * (value2 + 1.0f);
  value2 = smooth_step(value2);

  return value1 * value2;
}

static inline bool get_tree(seed_t seed, ivec2_t position)
{
  return noise_random2i(seed, position) < 0.005f;
}

static inline bool get_cave(seed_t seed, ivec3_t position)
{
  // Reference: https://blog.danol.cz/voxel-cave-generation-using-3d-perlin-noise-isosurfaces/
  float threshold = lerpf(0.0f, 0.1f, 1.0f/(1.0f+expf(position.z/1000.0f)));
  for(unsigned i=0; i<2; ++i)
  {
    float value = noise_perlin3_ex(seed_next(&seed), ivec3_as_fvec3(position), 0.02f, 1.5f, 0.3f, 4);
    if(fabs(value) > threshold)
      return false;
  }
  return true;
}

static inline uint8_t get_block(seed_t seed, ivec3_t position, float height)
{
  int height1 = floorf(height);
  int height2 = height1 + 1;

  if(position.z < height1)
    return get_cave(seed, position) ? BLOCK_EMPTY : BLOCK_STONE;

  if(position.z < height2)
    return BLOCK_GRASS;

  if(position.z < WATER_HEIGHT)
    return BLOCK_WATER;

  if(position.z >= ETHER_HEIGHT)
    return BLOCK_ETHER;

  return BLOCK_EMPTY;
}

static uint8_t TREE[6][5][5] = {
  {
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_LOG,   BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
  },
  {
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_LOG,   BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
  },
  {
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_LOG,   BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
  },
  {
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LOG,   BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
  },
  {
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LOG,   BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
    {BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, },
  },
  {
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_LEAVE, BLOCK_EMPTY, },
    {BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, BLOCK_EMPTY, },
  },
};

static void place_block(uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH], ivec3_t position, uint8_t block)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH) return;
  if(position.y < 0 || position.y >= CHUNK_WIDTH) return;
  if(position.z < 0 || position.z >= CHUNK_WIDTH) return;
  blocks[position.z][position.y][position.x] = block;
}

static void place_tree(uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH], ivec3_t position)
{
  for(int z=0; z<6; ++z)
    for(int y=0; y<5; ++y)
      for(int x=0; x<5; ++x)
        if(TREE[z][y][x] != BLOCK_EMPTY)
          place_block(blocks, ivec3_add(position, ivec3(x-2, y-2, z)), TREE[z][y][x]);
}

void generate_blocks(seed_t seed, ivec3_t position, uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH])
{
  seed_t seed_height = seed ^ 0b0101010110101010111010110001001011011010111011010101111010101000;
  seed_t seed_block   = seed ^ 0b1011110101011101010110101010101010101001010101010100110101010001;
  seed_t seed_tree   = seed ^ 0b1010110101011111010110110101010101010101010101101010000101011101;

  float heights[CHUNK_WIDTH+4][CHUNK_WIDTH+4];
  bool  trees  [CHUNK_WIDTH+4][CHUNK_WIDTH+4];

  for(int y = -2; y<CHUNK_WIDTH+2; ++y)
    for(int x = -2; x<CHUNK_WIDTH+2; ++x)
    {
      ivec2_t local_position  = ivec2(x, y);
      ivec2_t global_position = ivec2_add(ivec2_mul_scalar(ivec2(position.x, position.y), CHUNK_WIDTH), local_position);
      heights[y+2][x+2] = get_height(seed_height, global_position);
      trees  [y+2][x+2] = get_tree(seed_tree, global_position);
    }

  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        ivec3_t local_position  = ivec3(x, y, z);
        ivec3_t global_position = ivec3_add(ivec3_mul_scalar(position, CHUNK_WIDTH), local_position);
        blocks[z][y][x] = get_block(seed_block, global_position, heights[y+2][x+2]);
      }

  for(int y = -2; y<CHUNK_WIDTH+2; ++y)
    for(int x = -2; x<CHUNK_WIDTH+2; ++x)
      if(trees[y+2][x+2])
      {
        int height = floorf(heights[y+2][x+2]) + 1;
        if(height >= WATER_HEIGHT)
          place_tree(blocks, ivec3(x, y, height - position.z * CHUNK_WIDTH));
      }
}

fvec3_t generate_spawn(seed_t seed)
{
  seed_t seed_height = seed ^ 0b0101010110101010111010110001001011011010111011010101111010101000;
  seed_t seed_block   = seed ^ 0b1011110101011101010110101010101010101001010101010100110101010001;
  seed_t seed_tree   = seed ^ 0b1010110101011111010110110101010101010101010101101010000101011101;

  (void)seed_height;
  (void)seed_block;
  (void)seed_tree;

  return fvec3(0.0f, 0.0f, get_height(seed_height, ivec2(0, 0))+10.0f);
}

