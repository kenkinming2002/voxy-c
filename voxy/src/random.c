#include "random.h"

#include <assert.h>

#define MULTIPLIER1 131421833123
#define CONSTANT1   1312379

#define MULTIPLIER2 232138911
#define CONSTANT2   7931307

void seed_combine(seed_t *seed, void *_data, size_t length)
{
  unsigned char *data = _data;
  for(size_t i=0; i<length; ++i)
    *seed = *seed * 31 + data[i];
}

seed_t seed_next(seed_t *seed)
{
  uint32_t a = (*seed >> 0)  & 0xFFFFFFFF;
  uint32_t b = (*seed >> 32) & 0xFFFFFFFF;

  a = a * MULTIPLIER1 + CONSTANT1;
  b = b * MULTIPLIER2 + CONSTANT2;

  *seed = 0;
  *seed |= ((((seed_t)a) >> 16) & 0xFFFF) << 48;
  *seed |= ((((seed_t)b) >> 16) & 0xFFFF) << 32;
  *seed |= ((((seed_t)a) >> 0)  & 0xFFFF) << 16;
  *seed |= ((((seed_t)b) >> 0)  & 0xFFFF) << 0;

  return *seed;
}

float seed_rand(seed_t *seed, float low, float high)
{
  assert(low <= high);
  return low + (float)seed_next(seed) / (float)SEED_MAX * (high - low);
}

int seed_randi(seed_t *seed, int low, int high)
{
  assert(low <= high);
  return low + seed_next(seed) % (high - low + 1);
}

