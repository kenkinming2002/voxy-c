#ifndef LIBCOMMON_MATH_AABB_H
#define LIBCOMMON_MATH_AABB_H

#include <libcommon/math/vector.h>
#include <libcommon/math/direction.h>

typedef struct { fvec1_t center; fvec1_t dimension; } aabb1_t;
typedef struct { fvec2_t center; fvec2_t dimension; } aabb2_t;
typedef struct { fvec3_t center; fvec3_t dimension; } aabb3_t;
typedef struct { fvec4_t center; fvec4_t dimension; } aabb4_t;

/// A convenient constructor.
static inline aabb1_t aabb1(fvec1_t center, fvec1_t dimension) { return (aabb1_t){ .center = center, .dimension = dimension }; }
static inline aabb2_t aabb2(fvec2_t center, fvec2_t dimension) { return (aabb2_t){ .center = center, .dimension = dimension }; }
static inline aabb3_t aabb3(fvec3_t center, fvec3_t dimension) { return (aabb3_t){ .center = center, .dimension = dimension }; }
static inline aabb4_t aabb4(fvec4_t center, fvec4_t dimension) { return (aabb4_t){ .center = center, .dimension = dimension }; }

/// Return the corner with minimal coordinates i.e. center - dimension * 0.5.
/// This relies on the fact that dimension is positive on all axis.
static inline fvec1_t aabb1_min_corner(aabb1_t aabb) { return fvec1_sub(aabb.center, fvec1_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec2_t aabb2_min_corner(aabb2_t aabb) { return fvec2_sub(aabb.center, fvec2_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec3_t aabb3_min_corner(aabb3_t aabb) { return fvec3_sub(aabb.center, fvec3_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec4_t aabb4_min_corner(aabb4_t aabb) { return fvec4_sub(aabb.center, fvec4_mul_scalar(aabb.dimension, 0.5f)); }

/// Return the corner with maximal coordinates i.e. center + dimension * 0.5.
/// This relies on the fact that dimension is positive on all axis.
static inline fvec1_t aabb1_max_corner(aabb1_t aabb) { return fvec1_add(aabb.center, fvec1_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec2_t aabb2_max_corner(aabb2_t aabb) { return fvec2_add(aabb.center, fvec2_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec3_t aabb3_max_corner(aabb3_t aabb) { return fvec3_add(aabb.center, fvec3_mul_scalar(aabb.dimension, 0.5f)); }
static inline fvec4_t aabb4_max_corner(aabb4_t aabb) { return fvec4_add(aabb.center, fvec4_mul_scalar(aabb.dimension, 0.5f)); }

/// Expand a box in the given direction, such that mathematically:
///   y is in new box if and only if there exists some x in original box and t
///   in [0,1] such that y = x + t * direction
static inline aabb1_t aabb1_expand(aabb1_t aabb, fvec1_t direction)
{
  fvec1_t min_corner = aabb1_min_corner(aabb);
  fvec1_t max_corner = aabb1_max_corner(aabb);

  for(int i=0; i<1; ++i)
    if(signbit(direction.values[i]))
      min_corner.values[i] += direction.values[i];
    else
      max_corner.values[i] += direction.values[i];

  aabb1_t result;
  result.center = fvec1_mul_scalar(fvec1_add(min_corner, max_corner), 0.5f);
  result.dimension = fvec1_sub(max_corner, min_corner);
  return result;
}

/// Expand a box in the given direction, such that mathematically:
///   y is in new box if and only if there exists some x in original box and t
///   in [0,1] such that y = x + t * direction
static inline aabb2_t aabb2_expand(aabb2_t aabb, fvec2_t direction)
{
  fvec2_t min_corner = aabb2_min_corner(aabb);
  fvec2_t max_corner = aabb2_max_corner(aabb);

  for(int i=0; i<2; ++i)
    if(signbit(direction.values[i]))
      min_corner.values[i] += direction.values[i];
    else
      max_corner.values[i] += direction.values[i];

  aabb2_t result;
  result.center = fvec2_mul_scalar(fvec2_add(min_corner, max_corner), 0.5f);
  result.dimension = fvec2_sub(max_corner, min_corner);
  return result;
}

/// Expand a box in the given direction, such that mathematically:
///   y is in new box if and only if there exists some x in original box and t
///   in [0,1] such that y = x + t * direction
static inline aabb3_t aabb3_expand(aabb3_t aabb, fvec3_t direction)
{
  fvec3_t min_corner = aabb3_min_corner(aabb);
  fvec3_t max_corner = aabb3_max_corner(aabb);

  for(int i=0; i<3; ++i)
    if(signbit(direction.values[i]))
      min_corner.values[i] += direction.values[i];
    else
      max_corner.values[i] += direction.values[i];

  aabb3_t result;
  result.center = fvec3_mul_scalar(fvec3_add(min_corner, max_corner), 0.5f);
  result.dimension = fvec3_sub(max_corner, min_corner);
  return result;
}

/// Expand a box in the given direction, such that mathematically:
///   y is in new box if and only if there exists some x in original box and t
///   in [0,1] such that y = x + t * direction
static inline aabb4_t aabb4_expand(aabb4_t aabb, fvec4_t direction)
{
  fvec4_t min_corner = aabb4_min_corner(aabb);
  fvec4_t max_corner = aabb4_max_corner(aabb);

  for(int i=0; i<4; ++i)
    if(signbit(direction.values[i]))
      min_corner.values[i] += direction.values[i];
    else
      max_corner.values[i] += direction.values[i];

  aabb4_t result;
  result.center = fvec4_mul_scalar(fvec4_add(min_corner, max_corner), 0.5f);
  result.dimension = fvec4_sub(max_corner, min_corner);
  return result;
}

static inline aabb3_t aabb3_face(aabb3_t aabb, direction_t direction)
{
  const unsigned axis = direction_axis(direction);
  aabb.center.values[axis] += direction_signf(direction) * 0.5f * aabb.dimension.values[axis];
  aabb.dimension.values[axis] = 0.0f;
  return aabb;
}

#endif // LIBCOMMON_MATH_AABB_H
