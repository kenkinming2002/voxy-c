#include <voxy/main_game/world_seed.h>

#include <time.h>

static seed_t world_seed;

void world_seed_set(seed_t seed) { world_seed = seed; }
seed_t world_seed_get(void) { return world_seed; }

void world_seed_generate(void)
{
  world_seed = time(NULL);
}

