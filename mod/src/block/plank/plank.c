#include "plank.h"
#include "mod.h"

#include "block/block.h"
#include "item/plank/plank.h"

static block_id_t plank_block_id;

void plank_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "plank";

  block_info.light_type = BLOCK_LIGHT_TYPE_OPAQUE;
  block_info.render_type = BLOCK_RENDER_TYPE_OPAQUE;
  block_info.physics_type = BLOCK_PHYSICS_TYPE_CUBE;

  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/plank.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/plank.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/plank.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/plank.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/plank.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/plank.png";

  block_info.on_create = plank_block_on_create;
  block_info.on_destroy = plank_block_on_destroy;

  plank_block_id = register_block_info(block_info);
}

block_id_t plank_block_id_get(void)
{
  return plank_block_id;
}

void plank_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void plank_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, plank_item_id_get());
}

