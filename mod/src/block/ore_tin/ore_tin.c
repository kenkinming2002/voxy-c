#include "ore_tin.h"
#include "mod.h"

#include "block/block.h"
#include "item/ore_tin/ore_tin.h"

static block_id_t ore_tin_block_id;

void ore_tin_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "ore_tin";
  block_info.type = BLOCK_TYPE_OPAQUE;
  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/ore_tin.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/ore_tin.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/ore_tin.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/ore_tin.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/ore_tin.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/ore_tin.png";

  block_info.on_create = ore_tin_block_on_create;
  block_info.on_destroy = ore_tin_block_on_destroy;

  ore_tin_block_id = register_block_info(block_info);
}

block_id_t ore_tin_block_id_get(void)
{
  return ore_tin_block_id;
}

void ore_tin_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void ore_tin_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, ore_tin_item_id_get());
}

