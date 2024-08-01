#include <voxy/scene/main_game/update/physics/swept.h>

/// Solve for the interval (*t1, t2) such that aabb1 intersect with aabb2 if aabb1
/// is translated by t * offset where t is in (*t1, *t2).
bool swept_aabb1(aabb1_t aabb1, aabb1_t aabb2, fvec1_t offset, float *t1, float *t2, bool *direction1, bool *direction2)
{
  const float a1 = aabb1_min_corner(aabb1).x;
  const float b1 = aabb1_max_corner(aabb1).x;

  const float a2 = aabb1_min_corner(aabb2).x;
  const float b2 = aabb1_max_corner(aabb2).x;

  const float v = offset.x;

  *direction1 = signbit(v);
  *direction2 = !signbit(v);

  if(v != 0.0f)
  {
    *t1 = (a2 - b1) / v;
    *t2 = (b2 - a1) / v;
    if(*t1 > *t2)
    {
      const float tmp = *t1;
      *t1 = *t2;
      *t2 = tmp;
    }
    return *t1 < *t2;
  }
  else
  {
    *t1 = -INFINITY;
    *t2 = +INFINITY;
    return b1 > a2 && b2 > a1;
  }
}

/// Compute contact area between aabb1 and aabb2, assuming they are touching
/// along omit_axis.
static float contact_surface(aabb3_t aabb1, aabb3_t aabb2, unsigned omit_axis)
{
  const fvec3_t as1 = aabb3_min_corner(aabb1);
  const fvec3_t bs1 = aabb3_max_corner(aabb1);

  const fvec3_t as2 = aabb3_min_corner(aabb2);
  const fvec3_t bs2 = aabb3_max_corner(aabb2);

  float result = 1.0f;
  for(unsigned axis=0; axis<3; ++axis)
    if(axis != omit_axis)
    {
      const float a1 = as1.values[axis];
      const float b1 = bs1.values[axis];

      const float a2 = as2.values[axis];
      const float b2 = bs2.values[axis];

      const float a = a1 > a2 ? a1 : a2;
      const float b = b1 < b2 ? b1 : b2;

      result *= b - a;
    }

  return result;
}

/// Solve for the interval (*t1, t2) such that aabb1 intersect with aabb2 if
/// aabb1 is translated by t * offset where t is in (*t1, *t2).
///
/// Unforuntately, we run into the "t-fighting" problem if we hit two aabbs at
/// the same time. For example:
///
///           -----------
///           |         |
///           |         |
///           |         |
///           |    E    |
///           |         |
///           |         |
///     ------*----------
///     |     |     |
///     |  A  |  B  |
///     |     |     |
///     -------------
///
/// Entity E will hit both A and B at t = 0.0. To disambiguate, we also compute
/// *s1 and *s2 which are the contact surface area at *t1 and *t2 respectively.
/// In the above case, the contact with A is a line and hence the area is zero,
/// whereas the contact area with B is a plane and hence the area is non-zero.
/// We prioritize collision with largest contact area.
bool swept_aabb3(aabb3_t aabb1, aabb3_t aabb2, fvec3_t offset, float *t1, float *t2, float *s1, float *s2, direction_t *direction1, direction_t *direction2)
{
  float ts1[3];
  float ts2[3];

  bool directions1[3];
  bool directions2[3];

  for(unsigned axis=0; axis<3; ++axis)
  {
    const aabb1_t sub_aabb1 = { .center = fvec1(aabb1.center.values[axis]), .dimension = fvec1(aabb1.dimension.values[axis]) };
    const aabb1_t sub_aabb2 = { .center = fvec1(aabb2.center.values[axis]), .dimension = fvec1(aabb2.dimension.values[axis]) };
    const fvec1_t sub_offset = fvec1(offset.values[axis]);
    if(!swept_aabb1(sub_aabb1, sub_aabb2, sub_offset, &ts1[axis], &ts2[axis], &directions1[axis], &directions2[axis]))
      return false;
  }

  *t1 = -INFINITY;
  *t2 = +INFINITY;
  for(unsigned axis=0; axis<3; ++axis)
  {
    if(*t1 < ts1[axis])
    {
      *t1 = ts1[axis];
      *direction1 = direction_from_sign_axis(directions1[axis], axis);
    }

    if(*t2 > ts2[axis])
    {
      *t2 = ts2[axis];
      *direction2 = direction_from_sign_axis(directions2[axis], axis);
    }
  }

  if(*t1 >= *t2)
    return false;

  const fvec3_t saved_center = aabb1.center;

  aabb1.center = fvec3_add(aabb1.center, fvec3_mul_scalar(offset, *t1));
  {
    *s1 = 1.0f;
    *s1 = contact_surface(aabb1, aabb2, direction_axis(*direction1));
  }
  aabb1.center = saved_center;

  aabb1.center = fvec3_add(aabb1.center, fvec3_mul_scalar(offset, *t2));
  {
    *s2 = 1.0f;
    *s2 = contact_surface(aabb1, aabb2, direction_axis(*direction2));
  }
  aabb1.center = saved_center;

  return true;
}

