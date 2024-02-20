#ifndef VOXY_MAIN_GAME_BLOCK_H
#define VOXY_MAIN_GAME_BLOCK_H

#include <stdint.h>

struct block
{
  uint8_t id;
  uint8_t ether       : 1;
  uint8_t light_level : 4;
};

#define BLOCK_NONE UINT8_MAX

#endif // VOXY_MAIN_GAME_BLOCK_H
