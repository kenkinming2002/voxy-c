#ifndef TYPES_HOTBAR_H
#define TYPES_HOTBAR_H

#include <types/item.h>
#include <stdint.h>

#define HOTBAR_SIZE 9

struct hotbar
{
  struct item items[HOTBAR_SIZE];
  uint8_t selection;
};

#endif // TYPES_HOTBAR_H

