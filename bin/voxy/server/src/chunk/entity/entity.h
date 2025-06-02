#ifndef CHUNK_ENTITY_ENTITY_H
#define CHUNK_ENTITY_ENTITY_H

#include <voxy/server/chunk/entity/entity.h>
#include <voxy/server/registry/entity.h>

#include <libmath/vector.h>

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

#endif // CHUNK_ENTITY_ENTITY_H
