#include "block.h"
#include "ids.h"

#include "entity/item/item.h"

#include <voxy/scene/main_game/states/chunks.h>

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

#include <voxy/core/log.h>

static item_id_t block_id_to_item_id(block_id_t block_id)
{
  if(block_id == BLOCK_ID_STONE) return ITEM_ID_STONE;
  if(block_id == BLOCK_ID_GRASS) return ITEM_ID_GRASS;
  if(block_id == BLOCK_ID_LOG)   return ITEM_ID_LOG;
  if(block_id == BLOCK_ID_LEAVE) return ITEM_ID_LEAVE;
  if(block_id == BLOCK_ID_LAMP)  return ITEM_ID_LAMP;
  assert(0 && "Unreachable");
}

void block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;
  (void)chunk;
  (void)position;
}

void block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position)
{
  (void)entity;

  struct entity item_entity;
  item_entity.position = ivec3_as_fvec3(local_position_to_global_position_i(position, chunk->position));
  item_entity.velocity = fvec3_zero();
  item_entity.rotation = fvec3_zero();
  item_entity.grounded = false;
  item_entity_init(&item_entity, (struct item) { .id = block_id_to_item_id(chunk_get_block_id(chunk, position)), .count = 1 });
  world_add_entity(item_entity);
}
