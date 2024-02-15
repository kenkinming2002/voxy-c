#ifndef TYPES_ENTITY_H
#define TYPES_ENTITY_H

#include <voxy/math/vector.h>
#include <voxy/math/transform.h>

#include <stdbool.h>

struct entity
{
  fvec3_t position;
  fvec3_t dimension;
  fvec3_t velocity;

  transform_t local_view_transform;

  bool grounded;
};

fvec3_t entity_view_position(const struct entity *entity);
fvec3_t entity_view_rotation(const struct entity *entity);
fvec3_t entity_view_direction(const struct entity *entity);

transform_t entity_view_transform(const struct entity *entity);

void entity_apply_impulse(struct entity *entity, fvec3_t impulse);
void entity_move(struct entity *entity, fvec2_t direction, float speed, float dt);
void entity_jump(struct entity *entity, float strength);

bool entity_ray_cast(struct entity *entity, float distance, ivec3_t *position, ivec3_t *normal);

#endif // TYPES_ENTITY_H
