#include "actions.h"
#include "block/empty/empty.h"

#include <voxy/scene/main_game/states/digger.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <libcommon/core/window.h>
#include <libcommon/math/direction.h>

void player_entity_update_actions(struct entity *entity, float dt)
{
  if(entity->health <= 0.0f)
    return;

  struct player_opaque *opaque = entity->opaque;
  if(opaque->inventory_opened)
    return;

  // Cooldown
  opaque->cooldown += dt;

  // Left click => break blocks
  {
    if(input_state(BUTTON_LEFT))
    {
      ivec3_t position;
      ivec3_t normal;
      if(entity_ray_cast(entity, 20.0f, &position, &normal))
      {
        digger_set_position(&g_digger, position);
        if(digger_dig(&g_digger, dt * PLAYER_DIG_SPEED))
          world_set_block(g_digger.position, empty_block_id_get(), entity);
      }
      return;
    }
    else
      digger_reset(&g_digger);
  }

  // Right click
  {
    if(input_state(BUTTON_RIGHT) && opaque->cooldown >= PLAYER_ACTION_COOLDOWN)
    {
      // Use item
      {
        struct item *item = &opaque->hotbar[opaque->hotbar_selection];
        if(item->id != ITEM_NONE)
        {
          const struct item_info *item_info = query_item_info(item->id);
          if(item_info->on_use)
            if(item_info->on_use(entity, item))
            {
              opaque->cooldown = 0.0f;
              return;
            }
        }
      }

      // Use block
      {
        ivec3_t position;
        ivec3_t normal;
        if(entity_ray_cast(entity, 20.0f, &position, &normal))
        {
          const ivec3_t global_position = position;
          const ivec3_t local_position = global_position_to_local_position_i(global_position);
          const ivec3_t chunk_position = get_chunk_position_i(global_position);

          struct chunk *chunk = world_get_chunk(chunk_position);
          if(chunk)
          {
            const block_id_t id = chunk_get_block_id(chunk, local_position);
            const struct block_info *info = query_block_info(id);
            if(info->on_use)
              if(info->on_use(entity, chunk, local_position))
              {
                opaque->cooldown = 0.0f;
                return;
              }
          }
        }
      }
    }
  }
}
