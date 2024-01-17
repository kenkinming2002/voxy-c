#ifndef VOXY_RANDOM_H
#define VOXY_RANDOM_H

#include "lin.h"

#include <stddef.h>
#include <stdint.h>
#include <limits.h>

typedef uint64_t seed_t;

#define SEED_MAX UINT64_MAX

void seed_combine(seed_t *seed, void *data, size_t length);
seed_t seed_next(seed_t *seed);

float seed_rand(seed_t *seed, float low, float high);
int seed_randi(seed_t *seed, int begin, int end);

struct vec2 seed_rand_vec2(seed_t *seed, struct vec2 low, struct vec2 high);
struct vec3 seed_rand_vec3(seed_t *seed, struct vec3 low, struct vec3 high);
struct vec4 seed_rand_vec4(seed_t *seed, struct vec4 low, struct vec4 high);

struct ivec2 seed_rand_ivec2(seed_t *seed, struct ivec2 low, struct ivec2 high);
struct ivec3 seed_rand_ivec3(seed_t *seed, struct ivec3 low, struct ivec3 high);
struct ivec4 seed_rand_ivec4(seed_t *seed, struct ivec4 low, struct ivec4 high);

#endif // VOXY_RANDOM_H
