#include <voxy/mod_interface.h>
#include <voxy/main_game/world.h>

#include <voxy/types/world.h>
#include <voxy/types/entity.h>

#include <assert.h>
#include <stdbool.h>

/// Who need json or whatever formats for your configuration language when you
/// got a turing complete language at your disposal?

#define ETHER_HEIGHT 128
#define WATER_HEIGHT 3

static void on_block_item_use(uint8_t item_id);

enum block_id
{
  BLOCK_EMPTY,
  BLOCK_ETHER,
  BLOCK_STONE,
  BLOCK_GRASS,
  BLOCK_LOG,
  BLOCK_LEAVE,
  BLOCK_WATER,
};

enum texture_id
{
  TEXTURE_STONE,
  TEXTURE_GRASS_BOTTOM,
  TEXTURE_GRASS_TOP,
  TEXTURE_GRASS_SIDE,
  TEXTURE_LOG_TOP_BOTTOM,
  TEXTURE_LOG_SIDE,
  TEXTURE_LEAVE,
  TEXTURE_WATER,
};

enum item_id
{
  ITEM_STONE,
  ITEM_GRASS,
  ITEM_LOG,
  ITEM_LEAVE,
};

const struct block_info block_infos[] = {
  [BLOCK_EMPTY] = { .name = "empty", .type = BLOCK_TYPE_INVISIBLE, .ether = false, .light_level = 0,  },
  [BLOCK_ETHER] = { .name = "ether", .type = BLOCK_TYPE_INVISIBLE, .ether = true,  .light_level = 15, },
  [BLOCK_STONE] = {
    .name           = "stone",
    .type           = BLOCK_TYPE_OPAQUE,
    .ether          = false,
    .light_level    = 0,
    .texture_left   = TEXTURE_STONE,
    .texture_right  = TEXTURE_STONE,
    .texture_back   = TEXTURE_STONE,
    .texture_front  = TEXTURE_STONE,
    .texture_bottom = TEXTURE_STONE,
    .texture_top    = TEXTURE_STONE
  },
  [BLOCK_GRASS] = {
    .name           = "grass",
    .type           = BLOCK_TYPE_OPAQUE,
    .ether          = false,
    .light_level    = 0,
    .texture_left   = TEXTURE_GRASS_SIDE,
    .texture_right  = TEXTURE_GRASS_SIDE,
    .texture_back   = TEXTURE_GRASS_SIDE,
    .texture_front  = TEXTURE_GRASS_SIDE,
    .texture_bottom = TEXTURE_GRASS_BOTTOM,
    .texture_top    = TEXTURE_GRASS_TOP,
  },
  [BLOCK_LOG] = {
    .name           = "log",
    .type           = BLOCK_TYPE_OPAQUE,
    .ether          = false,
    .light_level    = 0,
    .texture_left   = TEXTURE_LOG_SIDE,
    .texture_right  = TEXTURE_LOG_SIDE,
    .texture_back   = TEXTURE_LOG_SIDE,
    .texture_front  = TEXTURE_LOG_SIDE,
    .texture_bottom = TEXTURE_LOG_TOP_BOTTOM,
    .texture_top    = TEXTURE_LOG_TOP_BOTTOM
  },
  [BLOCK_LEAVE] = {
    .name           = "leave",
    .type           = BLOCK_TYPE_OPAQUE,
    .ether          = false,
    .light_level    = 0,
    .texture_left   = TEXTURE_LEAVE,
    .texture_right  = TEXTURE_LEAVE,
    .texture_back   = TEXTURE_LEAVE,
    .texture_front  = TEXTURE_LEAVE,
    .texture_bottom = TEXTURE_LEAVE,
    .texture_top    = TEXTURE_LEAVE
  },
  [BLOCK_WATER] = {
    .name           = "water",
    .type           = BLOCK_TYPE_TRANSPARENT,
    .ether          = false,
    .light_level    = 0,
    .texture_left   = TEXTURE_WATER,
    .texture_right  = TEXTURE_WATER,
    .texture_back   = TEXTURE_WATER,
    .texture_front  = TEXTURE_WATER,
    .texture_bottom = TEXTURE_WATER,
    .texture_top    = TEXTURE_WATER
  },
};

const struct block_texture_info block_texture_infos[] = {
  [TEXTURE_STONE]          = { .filepath = "assets/stone.png"          },
  [TEXTURE_GRASS_BOTTOM]   = { .filepath = "assets/grass_bottom.png"   },
  [TEXTURE_GRASS_TOP]      = { .filepath = "assets/grass_top.png"      },
  [TEXTURE_GRASS_SIDE]     = { .filepath = "assets/grass_side.png"     },
  [TEXTURE_LOG_TOP_BOTTOM] = { .filepath = "assets/log_top_bottom.png" },
  [TEXTURE_LOG_SIDE]       = { .filepath = "assets/log_side.png"       },
  [TEXTURE_LEAVE]          = { .filepath = "assets/leave.png"          },
  [TEXTURE_WATER]          = { .filepath = "assets/water.png"          },
};

const struct item_info item_infos[] = {
  [ITEM_STONE] = { .name = "stone", .texture_filepath = "assets/stone_item.png", .on_use = on_block_item_use, },
  [ITEM_GRASS] = { .name = "grass", .texture_filepath = "assets/grass_item.png", .on_use = on_block_item_use, },
  [ITEM_LOG]   = { .name = "log",   .texture_filepath = "assets/log_item.png",   .on_use = on_block_item_use, },
  [ITEM_LEAVE] = { .name = "leave", .texture_filepath = "assets/leave_item.png", .on_use = on_block_item_use, },
};

const size_t block_info_count         = sizeof block_infos         / sizeof block_infos        [0];
const size_t block_texture_info_count = sizeof block_texture_infos / sizeof block_texture_infos[0];
const size_t item_info_count          = sizeof item_infos / sizeof item_infos[0];

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

static uint8_t item_id_to_block_id(uint8_t item_id)
{
  switch(item_id)
  {
  case ITEM_STONE: return BLOCK_STONE;
  case ITEM_GRASS: return BLOCK_GRASS;
  case ITEM_LOG:   return BLOCK_LOG;
  case ITEM_LEAVE: return BLOCK_LEAVE;
  }
  assert(0);
}

static void on_block_item_use(uint8_t item_id)
{
  ivec3_t position;
  ivec3_t normal;
  if(world.player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world.player.base, &world, 20.0f, &position, &normal))
  {
    world.player.cooldown = 0.0f;

    uint8_t block_id = item_id_to_block_id(item_id);
    world_block_set_id(&world, ivec3_add(position, normal), block_id);
  }
}

