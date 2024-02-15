#include <voxy/main_game/player_spawn.h>
#include <voxy/main_game/world_seed.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>

#include <voxy/core/window.h>

#include <voxy/types/player.h>

#include <voxy/config.h>

#include <stdio.h>

static int mini(int a, int b)
{
  return a < b ? a : b;
}

void update_player_spawn(void)
{
  if(!player_spawned)
  {
    player_spawned = true;

    player.third_person = false;

    player.base.position                         = mod_generate_spawn(world_seed_get());
    player.base.velocity                         = fvec3_zero();
    player.base.dimension                        = PLAYER_DIMENSION;
    player.base.local_view_transform.translation = fvec3(0.0f, 0.0f, PLAYER_EYE_HEIGHT);
    player.base.local_view_transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);
    player.base.grounded                         = false;
    player.base.update                           = NULL;

    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        player.inventory.items[j][i].id    = ITEM_NONE;
        player.inventory.items[j][i].count = 0;
      }

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      player.hotbar.items[i].id    = ITEM_NONE;
      player.hotbar.items[i].count = 0;
    }

    player.item_held.id    = ITEM_NONE;
    player.item_held.count = 0;

    player.item_hovered = NULL;

    int count = mini(mod_item_info_count_get() * 2, HOTBAR_SIZE);
    for(int i=0; i<count; ++i)
    {
      player.hotbar.items[i].id = i % mod_item_info_count_get();
      player.hotbar.items[i].count = 8 * (i + 1);
    }

    player.cooldown = 0.0f;

    fprintf(stderr, "INFO: Spawning player at (%f, %f, %f) with %d items\n",
        player.base.position.x,
        player.base.position.y,
        player.base.position.z,
        count);
  }
}
