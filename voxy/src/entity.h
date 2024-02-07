#ifndef VOXY_ENTITY_H
#define VOXY_ENTITY_H

#include <voxy/math/vector.h>
#include "transform.h"

struct entity
{
  fvec3_t position;
  fvec3_t velocity;
  fvec3_t dimension;

  struct transform local_view_transform;
};

fvec3_t entity_view_position(const struct entity *entity);
fvec3_t entity_view_rotation(const struct entity *entity);
fvec3_t entity_view_direction(const struct entity *entity);

struct transform entity_view_transform(const struct entity *entity);

#endif // VOXY_ENTITY_H
