#ifndef LIBCOMMON_MATH_DIRECTION_H
#define LIBCOMMON_MATH_DIRECTION_H

#include <libcommon/math/vector.h>

/// Enum of 6 axial direction
typedef enum {
  DIRECTION_LEFT,
  DIRECTION_RIGHT,
  DIRECTION_BACK,
  DIRECTION_FRONT,
  DIRECTION_BOTTOM,
  DIRECTION_TOP,
  DIRECTION_COUNT,
} direction_t;

direction_t direction_from_sign_axis(bool sign, unsigned axis);

/// Return sign of direction i.e. true if direction is in the positive direction and false otherwise.
bool direction_sign(direction_t direction);

/// Return sign of direction i.e. 1 if direction is in the positive direction and -1 otherwise.
int direction_signi(direction_t direction);

/// Return sign of direction i.e. 1.0 if direction is in the positive direction and -1.0 otherwise.
float direction_signf(direction_t direction);

/// Return the index for the axis corresponding to the given direction.
unsigned direction_axis(direction_t direction);

/// Convert direction to a vector.
ivec3_t direction_as_ivec(direction_t direction);

/// Convert direction to a vector.
fvec3_t direction_as_fvec(direction_t direction);

#endif // LIBCOMMON_MATH_DIRECTION_H
