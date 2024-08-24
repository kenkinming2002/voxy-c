#include "ore_copper.h"
#include "mod.h"

#include "block/block.h"
#include "item/ore_copper_block/ore_copper_block.h"

static block_id_t ore_copper_block_id;

void ore_copper_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "ore_copper";

  block_info.light_type = BLOCK_LIGHT_TYPE_OPAQUE;
  block_info.render_type = BLOCK_RENDER_TYPE_OPAQUE;
  block_info.physics_type = BLOCK_PHYSICS_TYPE_CUBE;

  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "bin/mod/assets/textures/ore_copper.png";
  block_info.textures[DIRECTION_RIGHT]  = "bin/mod/assets/textures/ore_copper.png";
  block_info.textures[DIRECTION_BACK]   = "bin/mod/assets/textures/ore_copper.png";
  block_info.textures[DIRECTION_FRONT]  = "bin/mod/assets/textures/ore_copper.png";
  block_info.textures[DIRECTION_BOTTOM] = "bin/mod/assets/textures/ore_copper.png";
  block_info.textures[DIRECTION_TOP]    = "bin/mod/assets/textures/ore_copper.png";

  block_info.on_create = ore_copper_block_on_create;
  block_info.on_destroy = ore_copper_block_on_destroy;

  ore_copper_block_id = register_block_info(block_info);
}

block_id_t ore_copper_block_id_get(void)
{
  return ore_copper_block_id;
}

void ore_copper_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void ore_copper_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, ore_copper_block_item_id_get());
}

