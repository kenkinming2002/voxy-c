#include <main_game/world.h>

#include <types/world.h>

#include <time.h>
#include <stdio.h>

struct world *main_game_world_get()
{
  static int          world_initialized;
  static struct world world;
  if(!world_initialized)
  {
    seed_t seed = time(NULL);

    fprintf(stderr, "INFO: Initializing world with seed %zu\n", seed);

    // Note: All other fields of world are zero-initialized which is good enough
    world.seed = seed;
    world_initialized = true;
  }
  return &world;
}
