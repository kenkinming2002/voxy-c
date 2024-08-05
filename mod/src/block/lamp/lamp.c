#include "lamp.h"
#include "mod.h"

#include "block/block.h"
#include "item/lamp/lamp.h"

static block_id_t lamp_block_id;

void lamp_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "lamp";
  block_info.type = BLOCK_TYPE_OPAQUE;
  block_info.light_level = 14;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/lamp.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/lamp.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/lamp.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/lamp.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/lamp.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/lamp.png";

  block_info.on_create = lamp_block_on_create;
  block_info.on_destroy = lamp_block_on_destroy;

  lamp_block_id = register_block_info(block_info);
}

block_id_t lamp_block_id_get(void)
{
  return lamp_block_id;
}

void lamp_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void lamp_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, lamp_item_id_get());
}

