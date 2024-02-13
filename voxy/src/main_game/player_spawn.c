#include <voxy/main_game/player_spawn.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/mod.h>

#include <voxy/core/window.h>

#include <voxy/types/world.h>
#include <voxy/types/player.h>

#include <voxy/config.h>

#include <stdio.h>

static int mini(int a, int b)
{
  return a < b ? a : b;
}

void update_player_spawn(void)
{
  if(!world.player_spawned)
  {
    world.player_spawned = true;

    world.player.third_person = false;

    world.player.base.position                         = mod_generate_spawn(world.seed);
    world.player.base.velocity                         = fvec3_zero();
    world.player.base.dimension                        = PLAYER_DIMENSION;
    world.player.base.local_view_transform.translation = fvec3(0.0f, 0.0f, PLAYER_EYE_HEIGHT);
    world.player.base.local_view_transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);
    world.player.base.grounded                         = false;

    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        world.player.inventory.items[j][i].id    = ITEM_NONE;
        world.player.inventory.items[j][i].count = 0;
      }

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      world.player.hotbar.items[i].id    = ITEM_NONE;
      world.player.hotbar.items[i].count = 0;
    }

    world.player.item_held.id    = ITEM_NONE;
    world.player.item_held.count = 0;

    world.player.item_hovered = NULL;

    int count = mini(mod_item_info_count_get() * 2, HOTBAR_SIZE);
    for(int i=0; i<count; ++i)
    {
      world.player.hotbar.items[i].id = i % mod_item_info_count_get();
      world.player.hotbar.items[i].count = 8 * (i + 1);
    }

    world.player.cooldown = 0.0f;

    fprintf(stderr, "INFO: Spawning player at (%f, %f, %f) with %d items\n",
        world.player.base.position.x,
        world.player.base.position.y,
        world.player.base.position.z,
        count);
  }
}
