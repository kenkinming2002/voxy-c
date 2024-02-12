#ifndef TYPES_INVENTORY_H
#define TYPES_INVENTORY_H

#include <voxy/types/item.h>
#include <stdbool.h>

#define INVENTORY_SIZE_HORIZONTAL 9
#define INVENTORY_SIZE_VERTICAL   5

struct inventory
{
  struct item items[INVENTORY_SIZE_VERTICAL][INVENTORY_SIZE_HORIZONTAL];
  bool opened;
};

#endif // TYPES_INVENTORY_H
