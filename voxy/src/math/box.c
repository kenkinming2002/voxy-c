#include <voxy/math/box.h>

box_t box(fvec3_t center, fvec3_t dimension)
{
  return (box_t){
    .center = center,
    .dimension = dimension,
  };
}

fvec3_t box_min_corner(box_t box)
{
  return fvec3_sub(box.center, fvec3_mul_scalar(box.dimension, 0.5f));
}

fvec3_t box_max_corner(box_t box)
{
  return fvec3_add(box.center, fvec3_mul_scalar(box.dimension, 0.5f));
}

box_t box_expand(box_t box, fvec3_t direction)
{
  fvec3_t min_corner = box_min_corner(box);
  fvec3_t max_corner = box_max_corner(box);

  for(int i=0; i<3; ++i)
    if(signbit(direction.values[i]))
      min_corner.values[i] += direction.values[i];
    else
      max_corner.values[i] += direction.values[i];

  box_t result;
  result.center  = fvec3_mul_scalar(fvec3_add(min_corner, max_corner), 0.5f);
  result.dimension = fvec3_sub(max_corner, min_corner);
  return result;
}
