#ifndef CHUNK_ENTITY_ENTITY_H
#define CHUNK_ENTITY_ENTITY_H

#include <voxy/client/registry/entity.h>
#include <voxy/client/entity/entity.h>

#include <libmath/vector.h>

#include <stdbool.h>

struct voxy_entity
{
  bool alive;

  voxy_entity_id_t id;

  fvec3_t position;
  fvec3_t rotation;
};

#endif // CHUNK_ENTITY_ENTITY_H
