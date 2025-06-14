#ifndef VOXY_SERVER_CHUNK_ENTITY_MANAGER_H
#define VOXY_SERVER_CHUNK_ENTITY_MANAGER_H

#include <voxy/server/registry/entity.h>

#include "entity.h"

#include <voxy/server/export.h>

#include <libmath/vector.h>
#include <libnet/server.h>

#include <stdint.h>

struct voxy_entity_manager;

/// Spawn/despawn entity.
VOXY_SERVER_EXPORT entity_handle_t voxy_entity_spawn(voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque);
VOXY_SERVER_EXPORT void voxy_entity_despawn(entity_handle_t handle);

/// Get pointer to entity from handle.
///
/// Since entities are stored in a dynamic array under the hood, the returned
/// pointer are invalidated upon spawn/despawn.
VOXY_SERVER_EXPORT struct voxy_entity *voxy_entity_get(entity_handle_t handle);

#endif // VOXY_SERVER_CHUNK_ENTITY_MANAGER_H
