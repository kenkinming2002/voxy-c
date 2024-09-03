#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <libnet/server.h>

#include "entity/manager.h"

void player_manager_update(float dt, libnet_server_t server, struct entity_manager *entity_manager);

void player_manager_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy, struct entity_manager *entity_manager);
void player_manager_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy, struct entity_manager *entity_manager);
void player_manager_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *_message);

#endif // PLAYER_MANAGER_H
