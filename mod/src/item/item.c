#include "item.h"

#include <voxy/scene/main_game/states/chunks.h>

void item_on_use_place_block(struct entity *entity, struct item *item, block_id_t block_id)
{
  ivec3_t position;
  ivec3_t normal;
  if(entity_ray_cast(entity, 20.0f, &position, &normal))
    if(item->count > 0)
    {
      world_set_block(ivec3_add(position, normal), block_id, entity);
      if(--item->count == 0)
        item->id = ITEM_NONE;
    }
}
