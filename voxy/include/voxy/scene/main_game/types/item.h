#ifndef VOXY_MAIN_GAME_TYPES_ITEM_H
#define VOXY_MAIN_GAME_TYPES_ITEM_H

#include <voxy/scene/main_game/types/registry.h>

#include <stdint.h>

struct item
{
  item_id_t id;
  uint8_t count;
};

#define ITEM_MAX_STACK 64

#endif // VOXY_MAIN_GAME_TYPES_ITEM_H
