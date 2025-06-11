#include "entity.h"

#include <stdlib.h>

void voxy_entity_destroy(struct voxy_entity entity)
{
  struct voxy_entity_info info = voxy_query_entity(entity.id);
  info.destroy_opaque(entity.opaque);
}

int voxy_entity_serialize(const struct voxy_entity *entity, struct sqlite3_utils_blob *blob)
{
  char *buf;
  size_t len;

  libserde_serializer_t serializer = libserde_serializer_create_mem(&buf, &len);
  if(!serializer)
    goto err_create;

  libserde_serializer_try_write(serializer, entity->id, err_write);
  libserde_serializer_try_write(serializer, entity->position, err_write);
  libserde_serializer_try_write(serializer, entity->rotation, err_write);
  libserde_serializer_try_write(serializer, entity->velocity, err_write);
  libserde_serializer_try_write(serializer, entity->grounded, err_write);

  const struct voxy_entity_info info = voxy_query_entity(entity->id);
  if(info.serialize_opaque(serializer, entity->opaque) != 0)
    goto err_write;

  libserde_serializer_destroy(serializer);

  blob->data = buf;
  blob->length = len;
  return 0;

err_write:
  libserde_serializer_destroy(serializer);
  free(buf);
err_create:
    return -1;
}

int voxy_entity_deserialize(struct voxy_entity *entity, const struct sqlite3_utils_blob *blob)
{
  libserde_deserializer_t deserializer = libserde_deserializer_create_mem(blob->data, blob->length);
  if(!deserializer)
    goto err_create;

  libserde_deserializer_try_read(deserializer, entity->id, err_read);
  libserde_deserializer_try_read(deserializer, entity->position, err_read);
  libserde_deserializer_try_read(deserializer, entity->rotation, err_read);
  libserde_deserializer_try_read(deserializer, entity->velocity, err_read);
  libserde_deserializer_try_read(deserializer, entity->grounded, err_read);

  const struct voxy_entity_info info = voxy_query_entity(entity->id);
  if(info.deserialize_opaque(deserializer, &entity->opaque) != 0)
    goto err_read;

  libserde_deserializer_destroy(deserializer);
  return 0;

err_read:
  libserde_deserializer_destroy(deserializer);
err_create:
  return -1;
}

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

