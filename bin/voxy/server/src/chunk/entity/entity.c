#include "entity.h"

transform_t voxy_entity_get_transform(const struct voxy_entity *entity)
{
  transform_t transform;
  transform.translation = entity->position;
  transform.rotation = entity->rotation;
  return transform;
}

fvec3_t voxy_entity_get_position(const struct voxy_entity *entity)
{
  return entity->position;
}

fvec3_t voxy_entity_get_rotation(const struct voxy_entity *entity)
{
  return entity->rotation;
}

void voxy_entity_set_transform(struct voxy_entity *entity, transform_t transform)
{
  entity->position = transform.translation;
  entity->rotation = transform.rotation;
}

void voxy_entity_set_position(struct voxy_entity *entity, fvec3_t position)
{
  entity->position = position;
}

void voxy_entity_set_rotation(struct voxy_entity *entity, fvec3_t rotation)
{
  entity->rotation = rotation;
}

void *voxy_entity_get_opaque(const struct voxy_entity *entity)
{
  return entity->opaque;
}

void voxy_entity_set_opaque(struct voxy_entity *entity, void *opaque)
{
  entity->opaque = opaque;
}

bool voxy_entity_is_grounded(const struct voxy_entity *entity)
{
  return entity->grounded;
}

void voxy_entity_apply_impulse(struct voxy_entity *entity, fvec3_t impulse)
{
  entity->velocity = fvec3_add(entity->velocity, impulse);
}

