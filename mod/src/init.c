#include "ids.h"
#include "generate.h"

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/update/generate.h>

#include <stdbool.h>

#define MOD "base"

void on_use_block_item(struct entity *entity, struct item *item);
void mod_init()
{
  // 1: Blocks
  BLOCK_ID_EMPTY = register_block_info((struct block_info){ .mod = MOD, .name = "empty", .type = BLOCK_TYPE_INVISIBLE, .ether = false, .light_level = 0,  });
  BLOCK_ID_ETHER = register_block_info((struct block_info){ .mod = MOD, .name = "ether", .type = BLOCK_TYPE_INVISIBLE, .ether = true,  .light_level = 15, });
  BLOCK_ID_STONE = register_block_info((struct block_info){
    .mod = MOD,
    .name = "stone",
    .type = BLOCK_TYPE_OPAQUE,
    .ether = false,
    .light_level = 0,
    .textures = {
      "assets/textures/stone.png",
      "assets/textures/stone.png",
      "assets/textures/stone.png",
      "assets/textures/stone.png",
      "assets/textures/stone.png",
      "assets/textures/stone.png"
    }
  });
  BLOCK_ID_GRASS = register_block_info((struct block_info){
    .mod = MOD,
    .name = "grass",
    .type = BLOCK_TYPE_OPAQUE,
    .ether = false,
    .light_level = 0,
    .textures = {
      "assets/textures/grass_side.png",
      "assets/textures/grass_side.png",
      "assets/textures/grass_side.png",
      "assets/textures/grass_side.png",
      "assets/textures/grass_bottom.png",
      "assets/textures/grass_top.png",
    }
  });
  BLOCK_ID_LOG = register_block_info((struct block_info){
    .mod = MOD,
    .name = "log",
    .type = BLOCK_TYPE_OPAQUE,
    .ether = false,
    .light_level = 0,
    .textures = {
      "assets/textures/log_side.png",
      "assets/textures/log_side.png",
      "assets/textures/log_side.png",
      "assets/textures/log_side.png",
      "assets/textures/log_top_bottom.png",
      "assets/textures/log_top_bottom.png",
    }
  });
  BLOCK_ID_LEAVE = register_block_info((struct block_info){
    .mod = MOD,
    .name = "leave",
    .type = BLOCK_TYPE_OPAQUE,
    .ether = false,
    .light_level = 0,
    .textures = {
      "assets/textures/leave.png",
      "assets/textures/leave.png",
      "assets/textures/leave.png",
      "assets/textures/leave.png",
      "assets/textures/leave.png",
      "assets/textures/leave.png",
    }
  });
  BLOCK_ID_WATER = register_block_info((struct block_info){
    .name = "water",
    .type = BLOCK_TYPE_TRANSPARENT,
    .ether = false,
    .light_level    = 0,
    .textures = {
      "assets/textures/water.png",
      "assets/textures/water.png",
      "assets/textures/water.png",
      "assets/textures/water.png",
      "assets/textures/water.png",
      "assets/textures/water.png"
    }
  });
  BLOCK_ID_LAMP = register_block_info((struct block_info){
    .name = "lamp",
    .type = BLOCK_TYPE_OPAQUE,
    .ether = false,
    .light_level = 15,
    .textures = {
      "assets/textures/lamp.png",
      "assets/textures/lamp.png",
      "assets/textures/lamp.png",
      "assets/textures/lamp.png",
      "assets/textures/lamp.png",
      "assets/textures/lamp.png"
    }
  });

  // 2: Items
  ITEM_ID_STONE = register_item_info((struct item_info){ .mod = MOD, .name = "stone", .texture = "assets/textures/stone_item.png", .on_use = on_use_block_item, });
  ITEM_ID_GRASS = register_item_info((struct item_info){ .mod = MOD, .name = "grass", .texture = "assets/textures/grass_item.png", .on_use = on_use_block_item, });
  ITEM_ID_LOG   = register_item_info((struct item_info){ .mod = MOD, .name = "log",   .texture = "assets/textures/log_item.png",   .on_use = on_use_block_item, });
  ITEM_ID_LEAVE = register_item_info((struct item_info){ .mod = MOD, .name = "leave", .texture = "assets/textures/leave_item.png", .on_use = on_use_block_item, });
  ITEM_ID_LAMP  = register_item_info((struct item_info){ .mod = MOD, .name = "lamp",  .texture = "assets/textures/leave_item.png", .on_use = on_use_block_item, });

  // 3: Callback
  register_generate_chunk_blocks(&base_generate_chunk_blocks);
  register_generate_player_spawn(&base_generate_player_spawn);
}

