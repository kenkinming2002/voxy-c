#include "noise.h"

#include <math.h>

float noise_random2(seed_t seed, struct vec2 position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}

float noise_random3(seed_t seed, struct vec3 position)
{
  seed_combine(&seed, &position.x, sizeof position.x);
  seed_combine(&seed, &position.y, sizeof position.y);
  seed_combine(&seed, &position.z, sizeof position.z);
  return (float)seed_next(&seed) / (float)SEED_MAX;
}


static struct vec2 gradient2(seed_t seed, int ix, int iy)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);

  for(;;)
  {
    struct vec2 vec = vec2(
      ((float)seed_next(&seed) / SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / SEED_MAX) * 2.0f - 1.0f);

    float length_squared = vec2_length_squared(vec);
    if(length_squared > 0.0f && length_squared <= 1.0f)
      return vec2_normalize(vec);
  }
}

static struct vec3 gradient3(seed_t seed, int ix, int iy, int iz)
{
  seed_combine(&seed, &ix, sizeof ix);
  seed_combine(&seed, &iy, sizeof iy);
  seed_combine(&seed, &iz, sizeof iz);

  for(;;)
  {
    struct vec3 vec = vec3(
      ((float)seed_next(&seed) / SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / SEED_MAX) * 2.0f - 1.0f,
      ((float)seed_next(&seed) / SEED_MAX) * 2.0f - 1.0f);

    float length_squared = vec3_length_squared(vec);
    if(length_squared > 0.0f && length_squared <= 1.0f)
      return vec3_normalize(vec);
  }
}

static float value2(seed_t seed, struct vec2 position, int ix, int iy)
{
  struct vec2 gradient = gradient2(seed, ix, iy);
  struct vec2 distance = vec2_sub(position, vec2(ix, iy));
  return vec2_dot(gradient, distance);
}

static float value3(seed_t seed, struct vec3 position, int ix, int iy, int iz)
{
  struct vec3 gradient = gradient3(seed, ix, iy, iz);
  struct vec3 distance = vec3_sub(position, vec3(ix, iy, iz));
  return vec3_dot(gradient, distance);
}

static float interpolate(float a, float b, float t)
{
  return a + (b - a) * t;
}

float noise_perlin2(seed_t seed, struct vec2 position)
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

float noise_perlin3(seed_t seed, struct vec3 position)
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
