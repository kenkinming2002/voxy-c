#ifndef ENTITY_ENTITY_H
#define ENTITY_ENTITY_H

#include "registry.h"

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>

struct entity
{
  entity_id_t id;

  fvec3_t position;
  fvec3_t rotation;
};

#endif // ENTITY_ENTITY_H
