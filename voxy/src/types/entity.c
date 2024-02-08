#include <types/entity.h>

fvec3_t entity_view_position(const struct entity *entity)
{
  return fvec3_add(entity->position, entity->local_view_transform.translation);
}

fvec3_t entity_view_rotation(const struct entity *entity)
{
  return entity->local_view_transform.rotation;
}

fvec3_t entity_view_direction(const struct entity *entity)
{
  return transform_forward(&entity->local_view_transform);
}

struct transform entity_view_transform(const struct entity *entity)
{
  struct transform result;
  result.translation = fvec3_add(entity->position, entity->local_view_transform.translation);
  result.rotation    = entity->local_view_transform.rotation;
  return result;
}

void entity_apply_impulse(struct entity *entity, fvec3_t impulse)
{
  entity->velocity = fvec3_add(entity->velocity, impulse);
}
