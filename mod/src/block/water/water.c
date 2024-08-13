#include "water.h"
#include "mod.h"

static block_id_t water_block_id;

void water_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "water";

  block_info.light_type = BLOCK_LIGHT_TYPE_PASSABLE;
  block_info.render_type = BLOCK_RENDER_TYPE_TRANSPARENT;
  block_info.physics_type = BLOCK_PHYSICS_TYPE_INVISIBLE;

  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/water.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/water.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/water.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/water.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/water.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/water.png";

  block_info.on_create = water_block_on_create;
  block_info.on_destroy = water_block_on_destroy;

  water_block_id = register_block_info(block_info);
}

block_id_t water_block_id_get(void)
{
  return water_block_id;
}

void water_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void water_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

