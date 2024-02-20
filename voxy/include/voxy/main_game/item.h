#ifndef VOXY_MAIN_GAME_ITEM_H
#define VOXY_MAIN_GAME_ITEM_H

#include <stdint.h>

struct item
{
  uint8_t id;
  uint8_t count;
};

#define ITEM_NONE UINT8_MAX
#define ITEM_MAX_STACK 64

#endif // VOXY_MAIN_GAME_ITEM_H
