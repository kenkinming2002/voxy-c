#ifndef VOXY_MAIN_GAME_TYPES_BLOCK_H
#define VOXY_MAIN_GAME_TYPES_BLOCK_H

#include <voxy/main_game/types/registry.h>
#include <stdint.h>

struct block
{
  block_id_t id;
  uint8_t    ether       : 1;
  uint8_t    light_level : 4;
};

#endif // VOXY_MAIN_GAME_TYPES_BLOCK_H
