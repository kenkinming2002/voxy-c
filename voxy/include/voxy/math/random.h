#ifndef VOXY_MATH_RANDOM_H
#define VOXY_MATH_RANDOM_H

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>

//////////////////
/// Interfaces ///
//////////////////

typedef uint64_t seed_t;

#define SEED_MAX UINT64_MAX

static inline void seed_combine(seed_t *seed, void *data, size_t length);
static inline seed_t seed_next(seed_t *seed);

static inline float seed_rand(seed_t *seed, float low, float high);
static inline int seed_randi(seed_t *seed, int begin, int end);

///////////////////////
/// Implementations ///
///////////////////////

#define VOXY_RANDOM_MULTIPLIER1 131421833123
#define VOXY_RANDOM_CONSTANT1   1312379

#define VOXY_RANDOM_MULTIPLIER2 232138911
#define VOXY_RANDOM_CONSTANT2   7931307

static inline void seed_combine(seed_t *seed, void *_data, size_t length)
{
  unsigned char *data = _data;
  for(size_t i=0; i<length; ++i)
    *seed = *seed * 31 + data[i];
}

static inline seed_t seed_next(seed_t *seed)
{
  uint32_t a = (*seed >> 0)  & 0xFFFFFFFF;
  uint32_t b = (*seed >> 32) & 0xFFFFFFFF;

  a = a * VOXY_RANDOM_MULTIPLIER1 + VOXY_RANDOM_CONSTANT1;
  b = b * VOXY_RANDOM_MULTIPLIER2 + VOXY_RANDOM_CONSTANT2;

  *seed = 0;
  *seed |= ((((seed_t)a) >> 16) & 0xFFFF) << 48;
  *seed |= ((((seed_t)b) >> 16) & 0xFFFF) << 32;
  *seed |= ((((seed_t)a) >> 0)  & 0xFFFF) << 16;
  *seed |= ((((seed_t)b) >> 0)  & 0xFFFF) << 0;

  return *seed;
}

static inline float seed_rand(seed_t *seed, float low, float high)
{
  assert(low <= high);
  return low + (float)seed_next(seed) / (float)SEED_MAX * (high - low);
}

static inline int seed_randi(seed_t *seed, int low, int high)
{
  assert(low <= high);
  return low + seed_next(seed) % (high - low + 1);
}


#endif // VOXY_MATH_RANDOM_H
