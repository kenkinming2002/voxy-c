#ifndef VOXY_NOISE_H
#define VOXY_NOISE_H

#include "lin.h"
#include "random.h"

float random2(seed_t seed, struct vec2 position);
float random3(seed_t seed, struct vec3 position);

float perlin2(seed_t seed, struct vec2 position);
float perlin3(seed_t seed, struct vec3 position);

#endif // VOXY_NOISE_H
