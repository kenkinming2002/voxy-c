#include "log.h"
#include "mod.h"

#include "block/block.h"
#include "item/log/log.h"

static block_id_t log_block_id;

void log_block_register(void)
{
  struct block_info block_info = {0};

  block_info.mod = MOD;
  block_info.name = "log";
  block_info.type = BLOCK_TYPE_OPAQUE;
  block_info.light_level = 0;

  block_info.textures[DIRECTION_LEFT]   = "mod/assets/textures/log_side.png";
  block_info.textures[DIRECTION_RIGHT]  = "mod/assets/textures/log_side.png";
  block_info.textures[DIRECTION_BACK]   = "mod/assets/textures/log_side.png";
  block_info.textures[DIRECTION_FRONT]  = "mod/assets/textures/log_side.png";
  block_info.textures[DIRECTION_BOTTOM] = "mod/assets/textures/log_top_bottom.png";
  block_info.textures[DIRECTION_TOP]    = "mod/assets/textures/log_top_bottom.png";

  block_info.on_create = log_block_on_create;
  block_info.on_destroy = log_block_on_destroy;

  log_block_id = register_block_info(block_info);
}

block_id_t log_block_id_get(void)
{
  return log_block_id;
}

void log_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void log_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  block_on_destroy_spawn_item(chunk, position, log_item_id_get());
}

