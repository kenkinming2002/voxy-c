#include <voxy/main_game/world.h>

#include <voxy/types/world.h>

#include <time.h>
#include <stdio.h>

struct world world;

void world_init(void)
{
  world.seed = time(NULL);
}

