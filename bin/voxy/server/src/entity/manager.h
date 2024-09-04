#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <voxy/server/entity/manager.h>

#include "entity.h"

#include <libnet/server.h>

#include <stdint.h>

/// Entity manager.
///
/// This takes care of allocating a unique handle for each entity and
/// synchronizing them over the network.
struct voxy_entity_manager
{
  DYNAMIC_ARRAY_DEFINE(,struct voxy_entity) entities;
  DYNAMIC_ARRAY_DEFINE(,entity_handle_t) orphans;
};

void voxy_entity_manager_init(struct voxy_entity_manager *entity_manager);
void entity_manager_fini(struct voxy_entity_manager *entity_manager);

/// Callbacks.
void voxy_entity_manager_update(struct voxy_entity_manager *entity_manager, libnet_server_t server);
void voxy_entity_manager_on_client_connected(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // ENTITY_MANAGER_H
