#include "ray_cast.h"

void ray_cast_init(struct ray_cast *ray_cast, fvec3_t position)
{
  ray_cast->fposition = position;
  ray_cast->iposition = fvec3_as_ivec3_round(ray_cast->fposition);
  ray_cast->distance  = 0.0f;
}

void ray_cast_step(struct ray_cast *ray_cast, fvec3_t direction)
{
  fvec3_t target = fvec3_add(ivec3_as_fvec3(ray_cast->iposition), fvec3_copysign(fvec3(0.5, 0.5f, 0.5f), direction));
  fvec3_t time   = fvec3_div(fvec3_sub(target, ray_cast->fposition), direction);

  int   min_index = -1;
  float min_value = INFINITY;
  for(int i=0; i<3; ++i)
    if(isfinite(time.values[i]) && min_value > time.values[i])
    {
      min_index = i;
      min_value = time.values[i];
    }

  ray_cast->iposition.values[min_index] += signbit(direction.values[min_index]) ? -1 : 1;
  ray_cast->fposition = fvec3_add(ray_cast->fposition, fvec3_mul_scalar(direction, min_value));
  ray_cast->distance += min_value;
}
