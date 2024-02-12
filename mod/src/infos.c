#include "ids.h"

#include <voxy/mod_interface.h>
#include <stdbool.h>

void on_block_item_use(uint8_t item_id);

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

