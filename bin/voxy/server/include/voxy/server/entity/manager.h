#ifndef VOXY_SERVER_ENTITY_MANAGER_H
#define VOXY_SERVER_ENTITY_MANAGER_H

#include "registry.h"

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>
#include <libnet/server.h>

#include <stdint.h>

typedef uint32_t entity_handle_t;
struct voxy_entity_manager;

/// Allocate/free entity.
///
/// In most cases, call entity_manager_spawn() and entity_manager_despawn()
/// instead since they take care of updating to all connected clients over the
/// network.
VOXY_SERVER_EXPORT entity_handle_t voxy_entity_manager_alloc(struct voxy_entity_manager *entity_manager);
VOXY_SERVER_EXPORT void voxy_entity_manager_free(struct voxy_entity_manager *entity_manager, entity_handle_t handle);

/// Spawn/despawn entity.
///
/// This takes care of updating all connected clients over the network.
VOXY_SERVER_EXPORT entity_handle_t voxy_entity_manager_spawn(struct voxy_entity_manager *entity_manager, entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server);
VOXY_SERVER_EXPORT void voxy_entity_manager_despawn(struct voxy_entity_manager *entity_manager, entity_handle_t handle, libnet_server_t server);

/// Get pointer to entity from handle.
///
/// Since entities are stored in a dynamic array under the hood, the returned
/// pointer are invalidated upon allocation/deallocation.
VOXY_SERVER_EXPORT struct voxy_entity *voxy_entity_manager_get(struct voxy_entity_manager *entity_manager, entity_handle_t handle);

#endif // VOXY_SERVER_ENTITY_MANAGER_H
