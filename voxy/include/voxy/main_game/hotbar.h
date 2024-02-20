#ifndef VOXY_MAIN_GAME_HOTBAR_H
#define VOXY_MAIN_GAME_HOTBAR_H

#include <voxy/main_game/item.h>
#include <stdint.h>

#define HOTBAR_SIZE 9

struct hotbar
{
  struct item items[HOTBAR_SIZE];
  uint8_t selection;
};

#endif // VOXY_MAIN_GAME_HOTBAR_H
