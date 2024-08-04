#include "ids.h"

#include <voxy/scene/main_game/config.h>
#include <voxy/math/noise.h>
#include <voxy/math/random.h>

#include <stdbool.h>

#define ETHER_HEIGHT 128
#define WATER_HEIGHT 2

static inline float get_river_factor(seed_t seed, ivec2_t position)
{ const size_t n = 3;
  float rivers[n];
  for(size_t i=0; i<n; ++i)
  {
    rivers[i] = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 1/500.0f, 2.0f, 0.4f, 8);
    rivers[i] = fabs(rivers[i]);
    rivers[i] = smooth_step_sigmoid(rivers[i], 0.008f, 0.001f);
    rivers[i] = 1.0f - rivers[i];
  }
  return smooth_min(n, rivers, 10.0f);
}

static inline float get_mountain_factor(seed_t seed, ivec2_t position)
{
  float mountain = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 0.008f, 2.0f, 0.3f, 8);
  mountain = (mountain + 1.0f) * 0.5f;
  mountain = smoother_step(mountain);
  return mountain;
}

static inline float get_bump_factor(seed_t seed, ivec2_t position)
{
  float scale = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 0.005f, 1.5f, 0.3f, 8);
  scale = (scale + 1.0f) * 0.5f;
  scale = ease_out(scale, 3.0f);
  scale = lerp(scale, 0.5f, 1.0f);

  float value = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 0.05f, 2.0f, 0.3f, 8);
  value = (value + 1.0f) * 0.5f;
  return scale * value;
}

static inline float get_height(seed_t seed, ivec2_t position)
{
  float river_factor = get_river_factor(seed ^ 0b0101010101111101010101010111011111010101010110101010101111010100, position);
  float mountain_factor = get_mountain_factor(seed ^ 0b0110111101010101010101101110101101110101101010101010010101010100, position);
  float bump_factor = get_bump_factor(seed ^ 0b1010101011010110111101010101010100001010110101010101010100010101, position);

  float flatness = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 0.001f, 1.1f, 0.3f, 4);
  flatness = (flatness + 1.0f) * 0.5f;
  flatness = smoother_step(flatness);

  float wetness = noise_perlin2_ex(seed_next(&seed), ivec2_as_fvec2(position), 0.001f, 1.1f, 0.3f, 4);
  wetness = (wetness + 1.0f) * 0.5f;
  wetness = smoother_step(wetness);

  // | flatness | wetness |
  // =======================================
  // | high     | high    | river plateau  | river + bump
  // | high     | low     | normal plateau |         bump
  // | low      |         | mountain range | mountain

  float mountain_blend_factor = powf(1.0f-flatness, 10.0f);
  float plateau_blend_factor  = powf(flatness, 10.0f);
  {
    float total = mountain_blend_factor + plateau_blend_factor;
    mountain_blend_factor /= total;
    plateau_blend_factor /= total;
  }
  float river_blend_factor = wetness;

  float river    = -10.0f * river_factor    * plateau_blend_factor * river_blend_factor;
  float bump     =  10.0f * bump_factor     * plateau_blend_factor;
  float mountain =  50.0f * mountain_factor * mountain_blend_factor;

  return river + mountain + bump;
}

static inline bool get_tree(seed_t seed, ivec2_t position)
{
  return noise_random2i(seed, position) < 0.005f;
}

static inline bool get_cave(seed_t seed, ivec3_t position)
{
  // Reference: https://blog.danol.cz/voxel-cave-generation-using-3d-perlin-noise-isosurfaces/
  float threshold = lerp(0.0f, 0.1f, 1.0f/(1.0f+expf(position.z/1000.0f)));
  for(unsigned i=0; i<2; ++i)
  {
    float value = noise_perlin3_ex(seed_next(&seed), ivec3_as_fvec3(position), 0.02f, 1.5f, 0.3f, 4);
    if(fabs(value) > threshold)
      return false;
  }
  return true;
}

static inline block_id_t get_block(seed_t seed, ivec3_t position, float height)
{
  int height1 = floorf(height);
  int height2 = height1 + 1;

  if(position.z < height2 && get_cave(seed, position))
    return BLOCK_ID_EMPTY;

  if(position.z < height1)
    return BLOCK_ID_STONE;

  if(position.z < height2)
    return BLOCK_ID_GRASS;

  if(position.z < WATER_HEIGHT)
    return BLOCK_ID_WATER;

  if(position.z >= ETHER_HEIGHT)
    return BLOCK_ID_ETHER;

  return BLOCK_ID_EMPTY;
}

static void place_block(block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH], ivec3_t position, block_id_t block)
{
  if(position.x < 0 || position.x >= CHUNK_WIDTH) return;
  if(position.y < 0 || position.y >= CHUNK_WIDTH) return;
  if(position.z < 0 || position.z >= CHUNK_WIDTH) return;
  block_ids[position.z][position.y][position.x] = block;
}

static void place_tree(block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH], ivec3_t position)
{
  const block_id_t TREE[6][5][5] = {
  {
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_LOG,   BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
  },
  {
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_LOG,   BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
  },
  {
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_LOG,   BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
  },
  {
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LOG,   BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
  },
  {
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LOG,   BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
    {BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, },
  },
  {
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_LEAVE, BLOCK_ID_EMPTY, },
    {BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, BLOCK_ID_EMPTY, },
  },
};

  for(int z=0; z<6; ++z)
    for(int y=0; y<5; ++y)
      for(int x=0; x<5; ++x)
        if(TREE[z][y][x] != BLOCK_ID_EMPTY)
          place_block(block_ids, ivec3_add(position, ivec3(x-2, y-2, z)), TREE[z][y][x]);
}

void base_generate_chunk_blocks(seed_t seed, ivec3_t position, block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH])
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
        block_ids[z][y][x] = get_block(seed_block, global_position, heights[y+2][x+2]);
      }

  for(int y = -2; y<CHUNK_WIDTH+2; ++y)
    for(int x = -2; x<CHUNK_WIDTH+2; ++x)
      if(trees[y+2][x+2])
      {
        int height = floorf(heights[y+2][x+2]) + 1;
        if(height >= WATER_HEIGHT)
          place_tree(block_ids, ivec3(x, y, height - position.z * CHUNK_WIDTH));
      }
}

fvec3_t base_generate_player_spawn(seed_t seed)
{
  seed_t seed_height = seed ^ 0b0101010110101010111010110001001011011010111011010101111010101000;
  seed_t seed_block   = seed ^ 0b1011110101011101010110101010101010101001010101010100110101010001;
  seed_t seed_tree   = seed ^ 0b1010110101011111010110110101010101010101010101101010000101011101;

  (void)seed_height;
  (void)seed_block;
  (void)seed_tree;

  return fvec3(0.0f, 0.0f, get_height(seed_height, ivec2(0, 0))+10.0f);
}

