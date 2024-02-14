#include <voxy/main_game/player_action.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>

#include <voxy/types/player.h>

#include <voxy/mod_interface.h>

#include <voxy/core/window.h>

#include <voxy/config.h>

void update_player_action(float dt)
{
  if(!player_spawned)
    return;

  if(player.inventory.opened)
    return;

  player.cooldown += dt;

  ivec3_t position;
  ivec3_t normal;

  if(input_state(BUTTON_LEFT) && player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&player.base, 20.0f, &position, &normal))
  {
    player.cooldown = 0.0f;

    int radius = input_state(KEY_CTRL) ? 2 : 0;
    for(int dz=-radius; dz<=radius; ++dz)
      for(int dy=-radius; dy<=radius; ++dy)
        for(int dx=-radius; dx<=radius; ++dx)
        {
          ivec3_t offset = ivec3(dx, dy, dz);
          if(ivec3_length_squared(offset) <= radius * radius)
            world_block_set(ivec3_add(position, offset), 0);
        }
  }

  if(input_state(BUTTON_RIGHT))
  {
    const struct item *item = &player.hotbar.items[player.hotbar.selection];
    if(item->id != ITEM_NONE)
      mod_item_info_get(item->id)->on_use(item->id);
  }
}
