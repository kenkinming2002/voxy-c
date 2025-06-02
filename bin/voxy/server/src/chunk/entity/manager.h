#ifndef CHUNK_ENTITY_MANAGER_H
#define CHUNK_ENTITY_MANAGER_H

#include <voxy/server/chunk/entity/manager.h>

#include <empty.h>

#include <libnet/server.h>

#include <stdint.h>

/// On start callback.
///
/// This is responsible for loading entitities from saved game data and need to
/// be called after mod initialization as it depends on all entities from mod
/// having been registered in the entity registry.
void voxy_entity_manager_start(void);

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_create(int64_t db_id, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque);

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_destroy(entity_handle_t handle);

/// Callbacks.
void voxy_entity_manager_update(void);
void voxy_entity_manager_on_client_connected(libnet_client_proxy_t client_proxy);

#endif // CHUNK_ENTITY_MANAGER_H
