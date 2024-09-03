#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "entity.h"

#include <libnet/server.h>

#include <stdint.h>

typedef uint32_t entity_handle_t;

struct entity_manager
{
  DYNAMIC_ARRAY_DEFINE(,struct entity) entities;
  DYNAMIC_ARRAY_DEFINE(,entity_handle_t) orphans;
};

void entity_manager_init(struct entity_manager *entity_manager);
void entity_manager_fini(struct entity_manager *entity_manager);

/// Allocate/free entity.
///
/// In most cases, call entity_manager_spawn() and entity_manager_despawn()
/// instead since they take care of updating to all connected clients over the
/// network.
entity_handle_t entity_manager_alloc(struct entity_manager *entity_manager);
void entity_manager_free(struct entity_manager *entity_manager, entity_handle_t handle);

/// Callbacks.
void entity_manager_update(struct entity_manager *entity_manager, libnet_server_t server);
void entity_manager_on_client_connected(struct entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

/// Get pointer to entity from handle.
///
/// Since entities are stored in a dynamic array under the hood, the returned
/// pointer are invalidated upon allocation/deallocation.
struct entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle);

/// Spawn/despawn entity.
///
/// This take care of updating to all connected clients over the network.
entity_handle_t entity_manager_spawn(struct entity_manager *entity_manager, entity_id_t id, fvec3_t position, fvec3_t rotation, libnet_server_t server);
void entity_manager_despawn(struct entity_manager *entity_manager, entity_handle_t handle, libnet_server_t server);

#endif // ENTITY_MANAGER_H
