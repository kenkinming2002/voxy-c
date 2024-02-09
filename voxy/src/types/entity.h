#ifndef TYPES_ENTITY_H
#define TYPES_ENTITY_H

#include <voxy/math/vector.h>
#include <transform.h>

#include <stdbool.h>

struct world;
struct resource_pack;

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

bool entity_ray_cast(struct entity *entity, struct world *world, struct resource_pack *resource_pack, float distance, ivec3_t *position, ivec3_t *normal);

#endif // TYPES_ENTITY_H
