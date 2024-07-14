#include "actions.h"

#include <voxy/scene/main_game/states/digger.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/core/window.h>
#include <voxy/math/direction.h>

void player_entity_update_actions(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  if(opaque->inventory_opened)
    return;

  // Left click => break blocks
  {
    if(input_state(BUTTON_LEFT))
    {
      ivec3_t position;
      ivec3_t normal;
      if(entity_ray_cast(entity, 20.0f, &position, &normal))
      {
        if(!ivec3_eql(g_digger.position, position))
        {
          g_digger.position = position;
          g_digger.damage = 0.0f;
        }

        g_digger.damage += dt * PLAYER_DIG_SPEED;
        if(g_digger.damage >= 1.0f)
          world_set_block(position, 0, entity); // FIXME: We are hardcoding the fact that empty block have id 0
        else
          world_invalidate_block(position);

      }
    }
    else if(g_digger.damage != 0.0f)
    {
      world_invalidate_block(g_digger.position);
      g_digger.position = ivec3_zero();
      g_digger.damage = 0.0f;
    }
  }

  // Right click => place blocks
  {
    opaque->cooldown += dt;
    if(input_state(BUTTON_RIGHT) && opaque->cooldown >= PLAYER_ACTION_COOLDOWN)
    {
      struct item *item = &opaque->hotbar[opaque->hotbar_selection];
      if(item->id != ITEM_NONE)
      {
        const struct item_info *item_info = query_item_info(item->id);
        if(item_info->on_use)
          item_info->on_use(entity, item);
      }
      opaque->cooldown = 0.0f;
    }
  }
}
