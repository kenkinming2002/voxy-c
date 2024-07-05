#include "ids.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

#include <voxy/math/vector.h>

#include <assert.h>

static block_id_t item_id_to_block_id(item_id_t item_id)
{
  if(item_id == ITEM_ID_STONE) return BLOCK_ID_STONE;
  if(item_id == ITEM_ID_GRASS) return BLOCK_ID_GRASS;
  if(item_id == ITEM_ID_LOG)   return BLOCK_ID_LOG;
  if(item_id == ITEM_ID_LEAVE) return BLOCK_ID_LEAVE;
  if(item_id == ITEM_ID_LAMP)  return BLOCK_ID_LAMP;
  assert(0 && "Unreachable");
}

void on_use_block_item(struct entity *entity, struct item *item)
{
  ivec3_t position;
  ivec3_t normal;
  if(entity_ray_cast(entity, 20.0f, &position, &normal))
    if(item->count > 0)
    {
      ivec3_t block_position = ivec3_add(position, normal);

      struct block *block = world_get_block(block_position);
      block->id = item_id_to_block_id(item->id);
      world_invalidate_block(block_position);

      if(--item->count == 0)
        item->id = ITEM_NONE;
    }
}

