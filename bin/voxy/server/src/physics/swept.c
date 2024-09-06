#include "swept.h"

#include <libcommon/utils/utils.h>
#include <libcommon/core/log.h>

int contact3_compare(struct contact3 contact1, struct contact3 contact2)
{
  if(contact1.time < contact2.time) return 1;
  if(contact1.time > contact2.time) return -1;

  if(contact1.area < contact2.area) return -1;
  if(contact1.area > contact2.area) return 1;

  return 0;
}

bool swept_aabb3(aabb3_t aabb1, aabb3_t aabb2, fvec3_t offset, struct contact3 *contact1, struct contact3 *contact2)
{
  const fvec3_t min_corner1 = aabb3_min_corner(aabb1);
  const fvec3_t min_corner2 = aabb3_min_corner(aabb2);

  const fvec3_t max_corner1 = aabb3_max_corner(aabb1);
  const fvec3_t max_corner2 = aabb3_max_corner(aabb2);

  bool initialized = false;

  unsigned axis1;
  unsigned axis2;

  // Check for collision times and normals along all axis.
  for(unsigned axis=0; axis<3; ++axis)
    if(offset.values[axis] != 0.0f)
    {
      const float t1 = (min_corner2.values[axis] - max_corner1.values[axis]) / offset.values[axis];
      const float t2 = (max_corner2.values[axis] - min_corner1.values[axis]) / offset.values[axis];

      if(!initialized || contact1->time < MIN(t1, t2))
      {
        axis1 = axis;
        contact1->time = MIN(t1, t2);
        contact1->normal = direction_as_fvec(direction_from_sign_axis(offset.values[axis] < 0.0f, axis));
      }

      if(!initialized || contact2->time > MAX(t1, t2))
      {
        axis2 = axis;
        contact2->time = MAX(t1, t2);
        contact2->normal = direction_as_fvec(direction_from_sign_axis(offset.values[axis] > 0.0f, axis));
      }

      initialized = true;
    }
    else if(max_corner1.values[axis] <= min_corner2.values[axis] || max_corner2.values[axis] <= min_corner1.values[axis])
        return false;

  // There only reason we are not initialized is if
  //  - We are intersecting on all axis.
  //  - Offset is zero so we never "un-intersect" ourselves.
  // Bails out.
  if(!initialized)
    return false;

  if(contact1->time >= contact2->time)
    return false;

  // Now comes the important part. We want to compute the contact surface area
  // at time of collision.
  contact1->area = 1.0f;
  contact2->area = 1.0f;
  for(unsigned axis = 0; axis < 3; ++axis)
  {
    if(axis != axis1)
    {
      const float a1 = min_corner1.values[axis] + contact1->time * offset.values[axis];
      const float b1 = max_corner1.values[axis] + contact1->time * offset.values[axis];

      const float a2 = min_corner2.values[axis];
      const float b2 = max_corner2.values[axis];

      contact1->area *= MAX(MIN(a1, b1), MIN(a2, b2)) - MIN(MAX(a1, b1), MAX(a2, b2));
    }

    if(axis != axis2)
    {
      const float a1 = min_corner1.values[axis] + contact2->time * offset.values[axis];
      const float b1 = max_corner1.values[axis] + contact2->time * offset.values[axis];

      const float a2 = min_corner2.values[axis];
      const float b2 = max_corner2.values[axis];

      contact2->area *= MAX(MIN(a1, b1), MIN(a2, b2)) - MIN(MAX(a1, b1), MAX(a2, b2));
    }
  }

  return true;
}

