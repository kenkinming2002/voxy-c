#ifndef TYPES_ITEM_H
#define TYPES_ITEM_H

#include <stdint.h>

struct item
{
  uint8_t id;
  uint8_t count;
};

#define ITEM_NONE UINT8_MAX
#define ITEM_MAX_STACK 64

#endif // TYPES_ITEM_H
