#ifndef TYPES_WORLD_H
#define TYPES_WORLD_H

#include <voxy/math/random.h>

#include <voxy/types/player.h>

struct world
{
  seed_t seed;

  bool                    player_spawned;
  struct player_entity    player;
};

#endif // TYPES_WORLD_H
