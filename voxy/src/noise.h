#ifndef VOXY_NOISE_H
#define VOXY_NOISE_H

#include "lin.h"
#include "random.h"

float noise_random2(seed_t seed, struct vec2 position);
float noise_random3(seed_t seed, struct vec3 position);

float noise_perlin2(seed_t seed, struct vec2 position);
float noise_perlin3(seed_t seed, struct vec3 position);

#endif // VOXY_NOISE_H
