#ifndef VOXY_MATH_DIRECTION_H
#define VOXY_MATH_DIRECTION_H

#include <voxy/math/vector.h>

/// Enum of 6 axial direction
enum direction {
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  DIRECTION_BACK,
  DIRECTION_FRONT,
  DIRECTION_BOTTOM,
  DIRECTION_TOP,
  DIRECTION_COUNT,
};

/// Convert direction to a vector.
fvec3_t direction_as_fvec(enum direction direction);
ivec3_t direction_as_ivec(enum direction direction);

#endif // VOXY_MATH_DIRECTION_H
