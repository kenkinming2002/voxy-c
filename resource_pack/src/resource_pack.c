#include <voxy/resource_pack.h>

#include <stdbool.h>

/// Who need json or whatever formats for your configuration language when you
/// got a turing complete language at your disposal?

#define ETHER_HEIGHT 128
#define WATER_HEIGHT 3

enum tile
{
  TILE_GRASS,
  TILE_STONE,
  TILE_WATER,
  TILE_EMPTY,
  TILE_ETHER,
};

enum texture
{
  TEXTURE_GRASS_BOTTOM,
  TEXTURE_GRASS_TOP,
  TEXTURE_GRASS_SIDE,
  TEXTURE_STONE,
  TEXTURE_WATER,
};

const struct block_info block_infos[] = {
  [TILE_GRASS] = { .name = "grass", .type = BLOCK_TYPE_OPAQUE,      .ether = false, .light_level = 0, .texture_left = TEXTURE_GRASS_SIDE, .texture_right = TEXTURE_GRASS_SIDE, .texture_back = TEXTURE_GRASS_SIDE, .texture_front = TEXTURE_GRASS_SIDE, .texture_bottom = TEXTURE_GRASS_BOTTOM, .texture_top = TEXTURE_GRASS_TOP },
  [TILE_STONE] = { .name = "stone", .type = BLOCK_TYPE_OPAQUE,      .ether = false, .light_level = 0, .texture_left = TEXTURE_STONE,      .texture_right = TEXTURE_STONE,      .texture_back = TEXTURE_STONE,      .texture_front = TEXTURE_STONE,      .texture_bottom = TEXTURE_STONE,        .texture_top = TEXTURE_STONE     },
  [TILE_WATER] = { .name = "water", .type = BLOCK_TYPE_TRANSPARENT, .ether = false, .light_level = 0, .texture_left = TEXTURE_WATER,      .texture_right = TEXTURE_WATER,      .texture_back = TEXTURE_WATER,      .texture_front = TEXTURE_WATER,      .texture_bottom = TEXTURE_WATER,        .texture_top = TEXTURE_WATER     },
  [TILE_EMPTY] = { .name = "empty", .type = BLOCK_TYPE_INVISIBLE,   .ether = false, .light_level = 0,  },
  [TILE_ETHER] = { .name = "ether", .type = BLOCK_TYPE_INVISIBLE,   .ether = true,  .light_level = 15, },
};

const struct block_texture_info block_texture_infos[] = {
  [TEXTURE_GRASS_BOTTOM] = { .filepath = "assets/grass_bottom.png" },
  [TEXTURE_GRASS_TOP]    = { .filepath = "assets/grass_top.png"    },
  [TEXTURE_GRASS_SIDE]   = { .filepath = "assets/grass_side.png"   },
  [TEXTURE_STONE]        = { .filepath = "assets/stone.png"        },
  [TEXTURE_WATER]        = { .filepath = "assets/water.png"        },
};

const size_t block_info_count         = sizeof block_infos         / sizeof block_infos        [0];
const size_t block_texture_info_count = sizeof block_texture_infos / sizeof block_texture_infos[0];

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

static inline uint8_t get_tile(seed_t seed, ivec3_t position, float height)
{
  float height1 = height;
  float height2 = height1 + 1.0f;

  if(position.z <= height1)
    return get_cave(seed, position) ? TILE_EMPTY : TILE_STONE;

  if(position.z <= height2)
    return TILE_GRASS;

  if(position.z <= WATER_HEIGHT)
    return TILE_WATER;

  if(position.z >= ETHER_HEIGHT)
    return TILE_ETHER;

  return TILE_EMPTY;
}

void generate_heights(seed_t seed, ivec2_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH])
{
  for(int y = 0; y<CHUNK_WIDTH; ++y)
    for(int x = 0; x<CHUNK_WIDTH; ++x)
    {
      ivec2_t real_position = ivec2_add(ivec2_mul_scalar(position, CHUNK_WIDTH), ivec2(x, y));
      heights[y][x] = get_height(seed, real_position);
    }
}

void generate_tiles(seed_t seed, ivec3_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH], uint8_t tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH])
{
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        ivec3_t real_position = ivec3_add(ivec3_mul_scalar(position, CHUNK_WIDTH), ivec3(x, y, z));
        tiles[z][y][x] = get_tile(seed, real_position, heights[y][x]);
      }
}

