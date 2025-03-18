#ifndef CHUNK_ENTITY_NETWORK_H
#define CHUNK_ENTITY_NETWORK_H

#include "entity.h"
#include "manager.h"

#include <libnet/server.h>

void voxy_entity_network_update(entity_handle_t handle, const struct voxy_entity *entity, libnet_server_t server, libnet_client_proxy_t proxy);
void voxy_entity_network_update_all(entity_handle_t handle, const struct voxy_entity *entity, libnet_server_t server);
void voxy_entity_network_remove_all(entity_handle_t handle, libnet_server_t server);

#endif // CHUNK_ENTITY_NETWORK_H
