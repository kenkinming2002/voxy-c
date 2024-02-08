#ifndef TYPES_ENTITY_H
#define TYPES_ENTITY_H

#include <voxy/math/vector.h>
#include <transform.h>

#include <stdbool.h>

struct entity
{
  fvec3_t position;
  fvec3_t dimension;
  fvec3_t velocity;

  struct transform local_view_transform;

  bool grounded;
};

void entity_apply_impulse(struct entity *entity, fvec3_t impulse);

fvec3_t entity_view_position(const struct entity *entity);
fvec3_t entity_view_rotation(const struct entity *entity);
fvec3_t entity_view_direction(const struct entity *entity);

struct transform entity_view_transform(const struct entity *entity);

#endif // TYPES_ENTITY_H
