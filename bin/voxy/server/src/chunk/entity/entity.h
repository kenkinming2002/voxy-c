#ifndef CHUNK_ENTITY_ENTITY_H
#define CHUNK_ENTITY_ENTITY_H

#include <voxy/server/chunk/entity/entity.h>

#include "registry/entity.h"

#include <libmath/vector.h>
#include <libcore/dynamic_array.h>

#include <stdbool.h>

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

DYNAMIC_ARRAY_DEFINE(voxy_entities, struct voxy_entity);

#endif // CHUNK_ENTITY_ENTITY_H
