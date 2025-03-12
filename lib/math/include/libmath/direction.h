#ifndef LIBMATH_DIRECTION_H
#define LIBMATH_DIRECTION_H

#include <libmath/vector.h>

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

/// Construct direction from sign and axis.
static inline direction_t direction_from_sign_axis(bool sign, unsigned axis);

/// Get the reverse direction.
static inline direction_t direction_reverse(direction_t direction);

/// Return sign of direction i.e. true if direction is in the positive direction and false otherwise.
static inline bool direction_sign(direction_t direction);

/// Return sign of direction i.e. 1 if direction is in the positive direction and -1 otherwise.
static inline int direction_signi(direction_t direction);

/// Return sign of direction i.e. 1.0 if direction is in the positive direction and -1.0 otherwise.
static inline float direction_signf(direction_t direction);

/// Return the index for the axis corresponding to the given direction.
static inline unsigned direction_axis(direction_t direction);

/// Convert direction to a vector.
static inline ivec3_t direction_as_ivec(direction_t direction);

/// Convert direction to a vector.
static inline fvec3_t direction_as_fvec(direction_t direction);

static inline direction_t direction_from_sign_axis(bool sign, unsigned axis)
{
  switch(axis)
  {
  case 0: return sign ? DIRECTION_RIGHT : DIRECTION_LEFT;
  case 1: return sign ? DIRECTION_FRONT : DIRECTION_BACK;
  case 2: return sign ? DIRECTION_TOP : DIRECTION_BOTTOM;
  default:
    assert(0 && "Unreachable");
  }
}

static inline direction_t direction_reverse(direction_t direction)
{
  return direction_from_sign_axis(!direction_sign(direction), direction_axis(direction));
}

static inline bool direction_sign(direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return false;
  case DIRECTION_RIGHT:  return true;
  case DIRECTION_BACK:   return false;
  case DIRECTION_FRONT:  return true;
  case DIRECTION_BOTTOM: return false;
  case DIRECTION_TOP:    return true;
  default:
    assert(0 && "Unreachable");
  }
}

static inline int direction_signi(direction_t direction)
{
  return direction_sign(direction) ? 1 : -1;
}

static inline float direction_signf(direction_t direction)
{
  return direction_signi(direction);
}

static inline unsigned direction_axis(direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:   return 0;
  case DIRECTION_RIGHT:  return 0;
  case DIRECTION_BACK:   return 1;
  case DIRECTION_FRONT:  return 1;
  case DIRECTION_BOTTOM: return 2;
  case DIRECTION_TOP:    return 2;
  default:
    assert(0 && "Unreachable");
  }
}

static inline ivec3_t direction_as_ivec(direction_t direction)
{
  ivec3_t result = ivec3_zero();
  result.values[direction_axis(direction)] = direction_signi(direction);
  return result;
}

static inline fvec3_t direction_as_fvec(direction_t direction)
{
  return ivec3_as_fvec3(direction_as_ivec(direction));
}


#endif // LIBMATH_DIRECTION_H
