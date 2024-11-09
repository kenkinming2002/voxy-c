#ifndef ENTITY_ENTITY_H
#define ENTITY_ENTITY_H

#include <voxy/server/entity/entity.h>

#include "registry.h"

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>

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

#endif // ENTITY_ENTITY_H
