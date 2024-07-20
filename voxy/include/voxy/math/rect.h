#ifndef VOXY_MATH_RECT_H
#define VOXY_MATH_RECT_H

#include "vector.h"

/// An axis aligned 3d quad where the axis of dimension as specified by
/// direction must be set to zero.
typedef struct {
  fvec3_t center;
  fvec3_t dimension;
  unsigned axis;
} rect_t;

#endif // VOXY_MATH_RECT_H
