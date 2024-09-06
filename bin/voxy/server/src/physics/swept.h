#ifndef PHYSICS_SWEPT_H
#define PHYSICS_SWEPT_H

#include <libcommon/math/aabb.h>
#include <libcommon/math/direction.h>

#include <stdbool.h>

struct contact3
{
  float time;
  float area;
  fvec3_t normal;
};

int contact3_compare(struct contact3 contact1, struct contact3 contact2);

///// Solve for the interval (*t1, t2) such that aabb1 intersect with aabb2 if
///// aabb1 is translated by t * offset where t is in (*t1, *t2).
/////
///// Unforuntately, we run into the "t-fighting" problem if we hit two aabbs at
///// the same time. For example:
/////
/////           -----------
/////           |         |
/////           |         |
/////           |         |
/////           |    E    |
/////           |         |
/////           |         |
/////     ------*----------
/////     |     |     |
/////     |  A  |  B  |
/////     |     |     |
/////     -------------
/////
///// Entity E will hit both A and B at t = 0.0. To disambiguate, we also compute
///// *s1 and *s2 which are the contact surface area at *t1 and *t2 respectively.
///// In the above case, the contact with A is a line and hence the area is zero,
///// whereas the contact area with B is a plane and hence the area is non-zero.
///// We prioritize collision with largest contact area.
bool swept_aabb3(aabb3_t aabb1, aabb3_t aabb2, fvec3_t offset, struct contact3 *contact1, struct contact3 *contact2);

#endif // PHYSICS_SWEPT_H
