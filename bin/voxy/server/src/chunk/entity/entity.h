#ifndef CHUNK_ENTITY_ENTITY_H
#define CHUNK_ENTITY_ENTITY_H

#include <voxy/server/chunk/entity/entity.h>
#include <voxy/server/registry/entity.h>

#include <libmath/vector.h>

#include <stdbool.h>

#include "sqlite3_utils.h"

struct voxy_entity
{
  bool alive;

  int64_t db_id;

  voxy_entity_id_t id;

  fvec3_t position;
  fvec3_t rotation;

  fvec3_t velocity;
  bool grounded;

  void *opaque;
};

/// Destroy an entity.
///
/// This is currently responsible for freeing the opaque blob by dispatching to
/// the implementation for the correct entity type.
void voxy_entity_destroy(struct voxy_entity entity);

/// Serialize/deserialize entity data.
///
/// This includes almost all fields of entity except alive and db_id which are
/// respectively allocator and database metadata that we are not responsible for
/// dealing with.
int voxy_entity_serialize(const struct voxy_entity *entity, struct sqlite3_utils_blob *blob);
int voxy_entity_deserialize(struct voxy_entity *entity, const struct sqlite3_utils_blob *blob);

#endif // CHUNK_ENTITY_ENTITY_H
