#ifndef ENTITY_ENTITY_H
#define ENTITY_ENTITY_H

#include <voxy/client/entity/entity.h>

#include "registry.h"

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>

#include <stdbool.h>

struct voxy_entity
{
  bool alive;

  entity_id_t id;

  fvec3_t position;
  fvec3_t rotation;
};

#endif // ENTITY_ENTITY_H
