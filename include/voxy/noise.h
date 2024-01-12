#ifndef VOXY_NOISE_H
#define VOXY_NOISE_H

#include <voxy/math.h>
#include <voxy/random.h>

float perlin2(seed_t seed, struct vec2 position);
float perlin3(seed_t seed, struct vec3 position);

#endif // VOXY_NOISE_H
