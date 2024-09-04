#include "entity.h"

fvec3_t voxy_entity_get_position(const struct voxy_entity *entity)
{
  return entity->position;
}

fvec3_t voxy_entity_get_rotation(const struct voxy_entity *entity)
{
  return entity->rotation;
}

void voxy_entity_set_position(struct voxy_entity *entity, fvec3_t position)
{
  entity->position = position;
}

void voxy_entity_set_rotation(struct voxy_entity *entity, fvec3_t rotation)
{
  entity->rotation = rotation;
}


