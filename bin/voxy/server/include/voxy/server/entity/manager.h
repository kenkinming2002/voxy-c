#ifndef VOXY_SERVER_ENTITY_MANAGER_H
#define VOXY_SERVER_ENTITY_MANAGER_H

#include <voxy/server/registry/entity.h>

#include "entity.h"
#include "database.h"

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>
#include <libnet/server.h>

#include <stdint.h>

struct voxy_entity_manager;

/// Spawn/despawn entity.
VOXY_SERVER_EXPORT entity_handle_t voxy_entity_manager_spawn(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server);
VOXY_SERVER_EXPORT void voxy_entity_manager_despawn(struct voxy_entity_manager *entity_manager, struct voxy_entity_database *entity_database, entity_handle_t handle, libnet_server_t server);

/// Get pointer to entity from handle.
///
/// Since entities are stored in a dynamic array under the hood, the returned
/// pointer are invalidated upon spawn/despawn.
VOXY_SERVER_EXPORT struct voxy_entity *voxy_entity_manager_get(struct voxy_entity_manager *entity_manager, entity_handle_t handle);

#endif // VOXY_SERVER_ENTITY_MANAGER_H
