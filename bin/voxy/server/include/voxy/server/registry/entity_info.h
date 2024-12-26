#ifndef VOXY_SERVER_ENTITY_INFO_H
#define VOXY_SERVER_ENTITY_INFO_H

#include <libcommon/math/vector.h>
#include <libserde/serializer.h>
#include <libserde/deserializer.h>
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

  /// Callback function to destroy entity opaque pointer.
  void(*destroy_opaque)(void *opaque);

  /// Serialization callback.
  ///
  /// Return non-zero value on error.
  int(*serialize_opaque)(libserde_serializer_t serializer, const void *opaque);

  /// Deserialization callback.
  ///
  /// The returned pointer will be cleaned up with destroy_opaque().
  ///
  /// Return NULL on error.
  void *(*deserialize_opaque)(libserde_deserializer_t deserializer);
};

#endif // VOXY_SERVER_ENTITY_INFO_H

