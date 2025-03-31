#ifndef CHUNK_ENTITY_MANAGER_H
#define CHUNK_ENTITY_MANAGER_H

#include <voxy/server/chunk/entity/manager.h>

#include <empty.h>

#include "chunk/manager.h"

#include "allocator.h"
#include "database.h"

#include <libnet/server.h>

#include <stdint.h>

struct loaded_chunk
{
  ivec3_t key;
  struct empty value;
};

/// Entity manager.
///
/// This takes care of allocating a unique handle for each entity and
/// synchronizing them over the network.
struct voxy_entity_manager
{
  struct entity_allocator allocator;
  struct loaded_chunk *loaded_chunks;
};

/// Initialization and deinitialization functions.
///
/// Only bare minimum is done.
void voxy_entity_manager_init(struct voxy_entity_manager *entity_manager);
void voxy_entity_manager_fini(struct voxy_entity_manager *entity_manager);

/// On start callback.
///
/// This is responsible for loading entitities from saved game data and need to
/// be called after mod initialization as it depends on all entities from mod
/// having been registered in the entity registry.
void voxy_entity_manager_start(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, libnet_server_t server);

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_manager_create_entity(struct voxy_entity_manager *entity_manager, int64_t db_id, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server);

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_manager_destroy_entity(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, entity_handle_t handle, libnet_server_t server);

/// Callbacks.
void voxy_entity_manager_update(struct voxy_entity_manager *entity_manager, struct voxy_entity_registry *entity_registry, struct voxy_entity_database *entity_database, struct voxy_chunk_manager *chunk_manager, libnet_server_t server);
void voxy_entity_manager_on_client_connected(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_ENTITY_MANAGER_H
