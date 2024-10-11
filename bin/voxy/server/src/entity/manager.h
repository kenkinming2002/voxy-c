#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "allocator.h"

#include "chunk/manager.h"
#include "hash_table/ivec3.h"

#include <libnet/server.h>

#include <stdint.h>

/// Entity manager.
///
/// This takes care of allocating a unique handle for each entity and
/// synchronizing them over the network.
struct voxy_entity_manager
{
  struct entity_allocator allocator;
  struct ivec3_hash_table loaded_chunks;
};

void voxy_entity_manager_init(struct voxy_entity_manager *entity_manager, libnet_server_t server);
void voxy_entity_manager_fini(struct voxy_entity_manager *entity_manager);

/// Create entity.
///
/// This takes care of allocating the entity and synchronizing the new state
/// over the network.
entity_handle_t voxy_entity_manager_create_entity(struct voxy_entity_manager *entity_manager, uint32_t cookie, voxy_entity_id_t id, fvec3_t position, fvec3_t rotation, void *opaque, libnet_server_t server);

/// Destroy entity.
///
/// This takes care of deallocating the entity and synchronizing the new state
/// over the network.
void voxy_entity_manager_destroy_entity(struct voxy_entity_manager *entity_manager, entity_handle_t handle, libnet_server_t server);

/// Callbacks.
void voxy_entity_manager_update(struct voxy_entity_manager *entity_manager, struct voxy_chunk_manager *chunk_manager, libnet_server_t server);
void voxy_entity_manager_on_client_connected(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // ENTITY_MANAGER_H
