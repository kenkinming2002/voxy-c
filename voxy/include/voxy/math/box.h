#ifndef VOXY_MATH_BOX_H
#define VOXY_MATH_BOX_H

#include "direction.h"
#include "rect.h"
#include "vector.h"

/// An axis aligned 3d box, where dimension must be positive on all axis.
typedef struct {
  fvec3_t center;
  fvec3_t dimension;
} box_t;

/// A convenient constructor.
box_t box(fvec3_t center, fvec3_t dimension);

/// Face of a box in the given direction.
rect_t box_face(box_t box, direction_t direction);

/// Return the corner with minimal coordinates i.e. center - dimension * 0.5.
/// This relies on the fact that dimension is positive on all axis.
fvec3_t box_min_corner(box_t box);

/// Return the corner with maximal coordinates i.e. center + dimension * 0.5.
/// This relies on the fact that dimension is positive on all axis.
fvec3_t box_max_corner(box_t box);

/// Expand a box in the given direction, such that mathematically:
///   y is in new box if and only if there exists some x in original box and t
///   in [0,1] such that y = x + t * direction
box_t box_expand(box_t box, fvec3_t direction);

#endif // VOXY_MATH_BOX_H
