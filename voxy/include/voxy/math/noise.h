#ifndef VOXY_MATH_NOISE_H
#define VOXY_MATH_NOISE_H

#include <voxy/math/vector.h>
#include <voxy/math/random.h>

//////////////////
/// Interfaces ///
//////////////////

static inline float noise_random2i(seed_t seed, ivec2_t position);
static inline float noise_random3i(seed_t seed, ivec3_t position);

static inline float noise_random2f(seed_t seed, fvec2_t position);
static inline float noise_random3f(seed_t seed, fvec3_t position);

static inline float noise_perlin2(seed_t seed, fvec2_t position);
static inline float noise_perlin3(seed_t seed, fvec3_t position);

static inline float noise_perlin2_ex(seed_t seed, fvec2_t position, float frequency, float lacunarity, float persistence, size_t octaves);
static inline float noise_perlin3_ex(seed_t seed, fvec3_t position, float frequency, float lacunarity, float persistence, size_t octaves);

static inline float lerp(float a, float b, float t);

static inline float ease_in(float x, float factor);
static inline float ease_out(float x, float factor);
static inline float smooth_step(float x);
static inline float smoother_step(float x);

///////////////////////
/// Implementations ///
///////////////////////

static inline float noise_random2i(seed_t seed, ivec2_t position)
{
  // FIXME: WTH
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed *= 0b1010110110101010101010101010101010101010;

  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed *= 0b1001010101010110101101011010101010101000;

  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed *= 0b0101011010101010101010110101010100101001;

  seed *= position.x;
  seed ^= 0b0010101011010101010101010101101010101011;
  seed *= position.y;
  seed ^= 0b0110101010110101010100110101011101010001;

  return (float)seed_next(&seed) / (float)SEED_MAX;
}

static inline float noise_random3i(seed_t seed, ivec3_t position)
{
  // FIXME: WTH
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  seed *= 0b1010110101111010101010101010101000010101;

  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  seed *= 0b1001010101101011101010101011010110101010;

  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  seed *= 0b0101010111010101010101010100010101010101;

  seed *= position.x;
  seed ^= 0b0010101011010101010101010101101010101011;
  seed *= position.y;
  seed ^= 0b1010101101110101111010111001010011010010;
  seed *= position.y;
  seed ^= 0b1010101010000001100101010101010111010010;

  return (float)seed_next(&seed) / (float)SEED_MAX;
}

static inline float noise_random2f(seed_t seed, fvec2_t position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}

static inline float noise_random3f(seed_t seed, fvec3_t position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}

static inline fvec2_t noise_perlin_gradient2(seed_t seed, int ix, int iy)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);

  float theta = seed_rand(&seed, 0.0f, 2.0f * M_PI);

  float x, y;
  sincosf(theta, &x, &y);

  return fvec2(x, y);
}

static inline fvec3_t noise_perlin_gradient3(seed_t seed, int ix, int iy, int iz)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);
  seed_combine(&seed, &iz, sizeof iz);

  float theta = seed_rand(&seed, 0.0f, 2.0f * M_PI);
  float z     = seed_rand(&seed, -1.0f, 1.0f);

  float x, y;
  sincosf(theta, &x, &y);

  float r = sqrtf(1.0f - z * z);
  x *= r;
  y *= r;

  return fvec3(x, y, z);
}

static inline float noise_perlin_value2(seed_t seed, fvec2_t position, int ix, int iy)
{
  fvec2_t gradient = noise_perlin_gradient2(seed, ix, iy);
  fvec2_t distance = fvec2_sub(position, fvec2(ix, iy));
  return fvec2_dot(gradient, distance);
}

static inline float noise_perlin_value3(seed_t seed, fvec3_t position, int ix, int iy, int iz)
{
  fvec3_t gradient = noise_perlin_gradient3(seed, ix, iy, iz);
  fvec3_t distance = fvec3_sub(position, fvec3(ix, iy, iz));
  return fvec3_dot(gradient, distance);
}

static inline float noise_perlin2(seed_t seed, fvec2_t position)
{
  int x0 = floorf(position.x), x1 = x0 + 1;
  int y0 = floorf(position.y), y1 = y0 + 1;

  float tx = position.x - (float)x0;
  float ty = position.y - (float)y0;

  return
    lerp(
      lerp(
        noise_perlin_value2(seed, position, x0, y0),
        noise_perlin_value2(seed, position, x1, y0),
        tx
      ),
      lerp(
        noise_perlin_value2(seed, position, x0, y1),
        noise_perlin_value2(seed, position, x1, y1),
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
    lerp(
      lerp(
        lerp(
          noise_perlin_value3(seed, position, x0, y0, z0),
          noise_perlin_value3(seed, position, x1, y0, z0),
          tx
        ),
        lerp(
          noise_perlin_value3(seed, position, x0, y1, z0),
          noise_perlin_value3(seed, position, x1, y1, z0),
          tx
        ),
        ty
      ),
      lerp(
        lerp(
          noise_perlin_value3(seed, position, x0, y0, z1),
          noise_perlin_value3(seed, position, x1, y0, z1),
          tx
        ),
        lerp(
          noise_perlin_value3(seed, position, x0, y1, z1),
          noise_perlin_value3(seed, position, x1, y1, z1),
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

static inline float lerp(float a, float b, float t)
{
  return a + (b - a) * t;
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

#endif // VOXY_MATH_NOISE_H
