#include "seed.h"

#include "libcore/format.h"
#include "libcore/fs.h"
#include "libcore/log.h"

#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>

static const char *world_directory;
static seed_t world_seed;

void world_init(const char *directory)
{
  world_directory = strdup(directory);

  const char *seed_path = tformat("%s/seed", directory);

  char *seed_data;
  size_t seed_length;
  if(read_file_all(seed_path, &seed_data, &seed_length) != 0)
  {
    seed_length = 32;
    seed_data = malloc(seed_length);

    srand(time(NULL));
    for(unsigned i=0; i<seed_length; ++i)
      while(!isalnum(seed_data[i] = rand()));

    LOG_INFO("Failed to find seed for block_group generator. Generated a new seed %.*s", (int)seed_length, seed_data);

    if(mkdir_recursive(world_directory) != 0)
    {
      LOG_INFO("Failed to create directory for block_group generator seed file: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }

    if(write_file_all(seed_path, seed_data, seed_length) != 0)
    {
      LOG_INFO("Failed to write block_group generator seed file: %s", strerror(errno));
      exit(EXIT_FAILURE);
    }
  }

  world_seed = 0b0101110101011010101110101101010101011010111010100011010100101010;
  seed_combine(&world_seed, seed_data, seed_length);
  seed_combine(&world_seed, seed_data, seed_length);

  LOG_INFO("Seed for block_group generator(string): %.*s", (int)seed_length, seed_data);
  LOG_INFO("Seed for block_group generator(integer): %zu", world_seed);

  free(seed_data);
}

const char *world_get_directory(void)
{
  return world_directory;
}

seed_t world_get_seed(void)
{
  return world_seed;
}
