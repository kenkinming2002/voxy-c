#ifndef VOXY_RANDOM_H
#define VOXY_RANDOM_H

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

typedef uint64_t seed_t;

#define SEED_MAX UINT64_MAX

void seed_combine(seed_t *seed, void *data, size_t length);
seed_t seed_next(seed_t *seed);

float seed_rand(seed_t *seed, float low, float high);
int seed_randi(seed_t *seed, int begin, int end);

#endif // VOXY_RANDOM_H
