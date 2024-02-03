#ifndef VOXY_RAY_CAST_H
#define VOXY_RAY_CAST_H

#include <voxy/math/vector.h>

struct ray_cast
{
  ivec3_t iposition;
  fvec3_t fposition;

  float distance;
};

void ray_cast_init(struct ray_cast *ray_cast, fvec3_t position);
void ray_cast_step(struct ray_cast *ray_cast, fvec3_t direction);

#endif // VOXY_RAY_CAST_H
