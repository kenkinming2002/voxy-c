#ifndef VOXY_MAIN_GAME_INVENTORY_H
#define VOXY_MAIN_GAME_INVENTORY_H

#include <voxy/main_game/item.h>
#include <stdbool.h>

#define INVENTORY_SIZE_HORIZONTAL 9
#define INVENTORY_SIZE_VERTICAL   5

struct inventory
{
  struct item items[INVENTORY_SIZE_VERTICAL][INVENTORY_SIZE_HORIZONTAL];
};

#endif // VOXY_MAIN_GAME_INVENTORY_H
