#ifndef VOXY_NOISE_H
#define VOXY_NOISE_H

#include "lin.h"
#include "random.h"

float noise_random2(seed_t seed, fvec2_t position);
float noise_random3(seed_t seed, fvec3_t position);

float noise_perlin2(seed_t seed, fvec2_t position);
float noise_perlin3(seed_t seed, fvec3_t position);

float noise_perlin2_ex(seed_t seed, fvec2_t position, float frequency, float lacunarity, float persistence, size_t octaves);
float noise_perlin3_ex(seed_t seed, fvec3_t position, float frequency, float lacunarity, float persistence, size_t octaves);

float ease_in(float x, float factor);
float ease_out(float x, float factor);
float smooth_step(float x);
float smoother_step(float x);


#endif // VOXY_NOISE_H
