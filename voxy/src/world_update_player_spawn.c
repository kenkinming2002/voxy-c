#include "world_update_player_spawn.h"

#include "world.h"
#include "resource_pack.h"

static int mini(int a, int b)
{
  return a < b ? a : b;
}

void world_update_player_spawn(struct world *world, struct resource_pack *resource_pack)
{
  if(!world->player.spawned)
  {
    world->player.spawned = true;
    world->player.transform.translation = resource_pack->generate_spawn(world->seed);
    world->player.transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);

    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        world->player.inventory.items[j][i].id    = ITEM_NONE;
        world->player.inventory.items[j][i].count = 0;
      }

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      world->player.hotbar.items[i].id    = ITEM_NONE;
      world->player.hotbar.items[i].count = 0;
    }

    world->player.inventory.opened = false;
    world->player.hotbar.selection = 0;
    world->player.item_hovered = NULL;
    world->player.item_held.id = ITEM_NONE;

    int count = mini(resource_pack->item_info_count * 2, HOTBAR_SIZE);
    for(int i=0; i<count; ++i)
    {
      world->player.hotbar.items[i].id = i % resource_pack->item_info_count;
      world->player.hotbar.items[i].count = 8 * (i + 1);
    }

    world->player.cooldown = 0.0f;

    printf("Spawning player at (%f, %f, %f) with %d items\n",
        world->player.transform.translation.x,
        world->player.transform.translation.y,
        world->player.transform.translation.z,
        count);
  }
}
