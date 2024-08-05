#include "stone.h"
#include "mod.h"

#include "block/block.h"
#include "item/stone/stone.h"

static block_id_t stone_block_id;

void stone_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "stone";
  block_info.type = BLOCK_TYPE_OPAQUE;
  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "assets/textures/stone.png";
  block_info.textures[DIRECTION_RIGHT]  = "assets/textures/stone.png";
  block_info.textures[DIRECTION_BACK]   = "assets/textures/stone.png";
  block_info.textures[DIRECTION_FRONT]  = "assets/textures/stone.png";
  block_info.textures[DIRECTION_BOTTOM] = "assets/textures/stone.png";
  block_info.textures[DIRECTION_TOP]    = "assets/textures/stone.png";

  block_info.on_create = stone_block_on_create;
  block_info.on_destroy = stone_block_on_destroy;

  stone_block_id = register_block_info(block_info);
}

block_id_t stone_block_id_get(void)
{
  return stone_block_id;
}

void stone_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void stone_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, stone_item_id_get());
}

