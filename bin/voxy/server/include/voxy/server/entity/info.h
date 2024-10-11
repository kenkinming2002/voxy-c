#ifndef VOXY_SERVER_ENTITY_INFO_H
#define VOXY_SERVER_ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>
#include <libcommon/math/vector.h>

#include <stdbool.h>

struct voxy_entity;
struct voxy_context;

struct voxy_entity_info
{
  const char *mod;
  const char *name;

  fvec3_t hitbox_offset;
  fvec3_t hitbox_dimension;

  /// Update callback.
  ///
  /// Called once per frame for each entity. Return false if entity should be
  /// removed.
  bool(*update)(struct voxy_entity *entity, float dt, const struct voxy_context *context);
};

DYNAMIC_ARRAY_DEFINE(voxy_entity_infos, struct voxy_entity_info);

#endif // VOXY_SERVER_ENTITY_INFO_H
