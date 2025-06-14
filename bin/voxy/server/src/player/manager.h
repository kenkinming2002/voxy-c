#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <voxy/server/player/manager.h>

#include <libnet/server.h>

void voxy_player_manager_on_client_connected(libnet_client_proxy_t client_proxy);
void voxy_player_manager_on_client_disconnected(libnet_client_proxy_t client_proxy);
void voxy_player_manager_on_message_received(libnet_client_proxy_t client_proxy, const struct libnet_message *message);

#endif // PLAYER_MANAGER_H
