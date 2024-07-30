#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/config.h>

#include <voxy/core/log.h>
#include <voxy/core/fs.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static seed_t world_seed;

void save_world_seed(void)
{
  char dirpath[] = WORLD_DIRPATH;
  if(mkdir_recursive(dirpath) != 0)
  {
    LOG_ERROR("Failed to save world seed: Failed to create directory: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(WORLD_DIRPATH "/seed", "wb");
  if(!f)
  {
    LOG_ERROR("Failed to save world seed: Failed to open file");
    exit(EXIT_FAILURE);
  }

  if(fwrite(&world_seed, sizeof world_seed, 1, f) != 1)
  {
    LOG_ERROR("Failed to save world seed: Failed to write to file");
    exit(EXIT_FAILURE);
  }
}

void load_world_seed(void)
{
  FILE *f = fopen(WORLD_DIRPATH "/seed", "rb");
  if(!f)
  {
    world_seed_generate();
    return;
  }

  if(fread(&world_seed, sizeof world_seed, 1, f) != 1)
  {
    world_seed_generate();
    return;
  }
}

void world_seed_set(seed_t seed) { world_seed = seed; }
seed_t world_seed_get(void) { return world_seed; }

void world_seed_generate(void)
{
  world_seed = time(NULL);
  LOG_INFO("World seed does not exist: Generated new seed: %zu", world_seed);
}

