#ifndef VOXY_NOISE_H
#define VOXY_NOISE_H

#include <voxy/math/vector.h>
#include <voxy/math/random.h>

//////////////////
/// Interfaces ///
//////////////////

static inline float noise_random2(seed_t seed, fvec2_t position);
static inline float noise_random3(seed_t seed, fvec3_t position);

static inline float noise_perlin2(seed_t seed, fvec2_t position);
static inline float noise_perlin3(seed_t seed, fvec3_t position);

static inline float noise_perlin2_ex(seed_t seed, fvec2_t position, float frequency, float lacunarity, float persistence, size_t octaves);
static inline float noise_perlin3_ex(seed_t seed, fvec3_t position, float frequency, float lacunarity, float persistence, size_t octaves);

static inline float ease_in(float x, float factor);
static inline float ease_out(float x, float factor);
static inline float smooth_step(float x);
static inline float smoother_step(float x);

///////////////////////
/// Implementations ///
///////////////////////

static inline float noise_random2(seed_t seed, fvec2_t position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}

static inline float noise_random3(seed_t seed, fvec3_t position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}


static inline fvec2_t gradient2(seed_t seed, int ix, int iy)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);

  for(;;)
  {
    fvec2_t vec = fvec2(
      ((float)seed_next(&seed) / (float)SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / (float)SEED_MAX) * 2.0f - 1.0f);

    float length_squared = fvec2_length_squared(vec);
    if(length_squared > 0.0f && length_squared <= 1.0f)
      return fvec2_normalize(vec);
  }
}

static inline fvec3_t gradient3(seed_t seed, int ix, int iy, int iz)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);
  seed_combine(&seed, &iz, sizeof iz);

  for(;;)
  {
    fvec3_t vec = fvec3(
      ((float)seed_next(&seed) / (float)SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / (float)SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / (float)SEED_MAX) * 2.0f - 1.0f);

    float length_squared = fvec3_length_squared(vec);
    if(length_squared > 0.0f && length_squared <= 1.0f)
      return fvec3_normalize(vec);
  }
}

static inline float value2(seed_t seed, fvec2_t position, int ix, int iy)
{
  fvec2_t gradient = gradient2(seed, ix, iy);
  fvec2_t distance = fvec2_sub(position, fvec2(ix, iy));
  return fvec2_dot(gradient, distance);
}

static inline float value3(seed_t seed, fvec3_t position, int ix, int iy, int iz)
{
  fvec3_t gradient = gradient3(seed, ix, iy, iz);
  fvec3_t distance = fvec3_sub(position, fvec3(ix, iy, iz));
  return fvec3_dot(gradient, distance);
}

static inline float interpolate(float a, float b, float t)
{
  return a + (b - a) * t;
}

static inline float noise_perlin2(seed_t seed, fvec2_t position)
{
  int x0 = floorf(position.x), x1 = x0 + 1;
  int y0 = floorf(position.y), y1 = y0 + 1;

  float tx = position.x - (float)x0;
  float ty = position.y - (float)y0;

  return
    interpolate(
      interpolate(
        value2(seed, position, x0, y0),
        value2(seed, position, x1, y0),
        tx
      ),
      interpolate(
        value2(seed, position, x0, y1),
        value2(seed, position, x1, y1),
        tx
      ),
      ty
    );
}

static inline float noise_perlin3(seed_t seed, fvec3_t position)
{
  int x0 = floorf(position.x), x1 = x0 + 1;
  int y0 = floorf(position.y), y1 = y0 + 1;
  int z0 = floorf(position.z), z1 = z0 + 1;

  float tx = position.x - x0;
  float ty = position.y - y0;
  float tz = position.z - z0;

  return
    interpolate(
      interpolate(
        interpolate(
          value3(seed, position, x0, y0, z0),
          value3(seed, position, x1, y0, z0),
          tx
        ),
        interpolate(
          value3(seed, position, x0, y1, z0),
          value3(seed, position, x1, y1, z0),
          tx
        ),
        ty
      ),
      interpolate(
        interpolate(
          value3(seed, position, x0, y0, z1),
          value3(seed, position, x1, y0, z1),
          tx
        ),
        interpolate(
          value3(seed, position, x0, y1, z1),
          value3(seed, position, x1, y1, z1),
          tx
        ),
        ty
      ),
      tz
    );
}

static inline float noise_perlin2_ex(seed_t seed, fvec2_t position, float frequency, float lacunarity, float persistence, size_t octaves)
{
  float amplitude = 1.0f;
  float max_value = 0.0f;
  float value     = 0.0f;
  for(size_t i=0; i<octaves; ++i)
  {
    max_value += amplitude;
    value += noise_perlin2(seed_next(&seed), fvec2_mul_scalar(position, frequency)) * amplitude;
    frequency *= lacunarity;
    amplitude *= persistence;
  }
  return value / max_value / (sqrtf(2.0f) / 4.0f);
}

static inline float noise_perlin3_ex(seed_t seed, fvec3_t position, float frequency, float lacunarity, float persistence, size_t octaves)
{
  float amplitude = 1.0f;
  float max_value = 0.0f;
  float value     = 0.0f;
  for(size_t i=0; i<octaves; ++i)
  {
    max_value += amplitude;
    value += noise_perlin3(seed_next(&seed), fvec3_mul_scalar(position, frequency)) * amplitude;
    frequency *= lacunarity;
    amplitude *= persistence;
  }
  return value / max_value / (sqrtf(3.0f) / 4.0f);
}

static inline float ease_in(float x, float factor)
{
  return (expf(x * factor) - 1.0f)/(expf(factor) - 1.0f);
}

static inline float ease_out(float x, float factor)
{
  return logf(x * factor + 1.0f) / logf(factor + 1.0f);
}

static inline float smooth_step(float x)
{
  return 3.0f * x * x
       - 2.0f * x * x * x;
}

static inline float smoother_step(float x)
{
  return 6.0f  * x * x * x * x * x
       - 15.0f * x * x * x * x
       + 10.0f * x * x * x;
}

#endif // VOXY_NOISE_H
