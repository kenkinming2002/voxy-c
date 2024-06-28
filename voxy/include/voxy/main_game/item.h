#ifndef VOXY_MAIN_GAME_ITEM_H
#define VOXY_MAIN_GAME_ITEM_H

#include <voxy/main_game/registry.h>

#include <stdint.h>

struct item
{
  item_id_t id;
  uint8_t count;
};

#define ITEM_MAX_STACK 64

#endif // VOXY_MAIN_GAME_ITEM_H
