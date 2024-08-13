#include "item.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/entity_query.h>

#include <libcommon/core/log.h>

bool item_on_use_place_block(struct entity *entity, struct item *item, block_id_t block_id)
{
  ivec3_t position;
  ivec3_t normal;
  if(entity_ray_cast(entity, 20.0f, &position, &normal))
    if(item->count > 0)
    {
      const ivec3_t target_position = ivec3_add(position, normal);

      const struct block_info *block_info = query_block_info(world_get_block_id(target_position));
      if(block_info->type == BLOCK_TYPE_OPAQUE)
        return false;

      struct entity **entities;
      size_t entity_count;
      world_query_entity(aabb3(ivec3_as_fvec3(target_position), fvec3(1.0f, 1.0f, 1.0f)), &entities, &entity_count);
      free(entities);
      if(entity_count != 0)
        return false;

      world_set_block(ivec3_add(position, normal), block_id, entity);
      if(--item->count == 0)
        item->id = ITEM_NONE;

      return true;
    }

  return false;
}
