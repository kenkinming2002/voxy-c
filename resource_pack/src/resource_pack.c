#include <voxy/resource_pack.h>

enum texture
{
  TEXTURE_GRASS_BOTTOM,
  TEXTURE_GRASS_TOP,
  TEXTURE_GRASS_SIDE,
  TEXTURE_STONE,
};

const struct block_info block_infos[] = {
  { .name = "grass", .texture_left = TEXTURE_GRASS_SIDE, .texture_right = TEXTURE_GRASS_SIDE, .texture_back = TEXTURE_GRASS_SIDE, .texture_front = TEXTURE_GRASS_SIDE, .texture_bottom = TEXTURE_GRASS_BOTTOM,  .texture_top = TEXTURE_GRASS_TOP },
  { .name = "stone", .texture_left = TEXTURE_STONE,      .texture_right = TEXTURE_STONE,      .texture_back = TEXTURE_STONE,      .texture_front = TEXTURE_STONE,      .texture_bottom = TEXTURE_STONE,         .texture_top = TEXTURE_STONE     },
};

const struct block_texture_info block_texture_infos[] = {
  [TEXTURE_GRASS_BOTTOM] = { .filepath = "assets/grass_bottom.png" },
  [TEXTURE_GRASS_TOP]    = { .filepath = "assets/grass_top.png"    },
  [TEXTURE_GRASS_SIDE]   = { .filepath = "assets/grass_side.png"   },
  [TEXTURE_STONE]        = { .filepath = "assets/stone.png"        },
};

const size_t block_info_count         = sizeof block_infos         / sizeof block_infos        [0];
const size_t block_texture_info_count = sizeof block_texture_infos / sizeof block_texture_infos[0];
