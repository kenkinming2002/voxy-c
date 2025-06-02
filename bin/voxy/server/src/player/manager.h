#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <voxy/server/player/manager.h>

#include <libnet/server.h>

void voxy_player_manager_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy);
void voxy_player_manager_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy);
void voxy_player_manager_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *message, const struct voxy_context *context);

#endif // PLAYER_MANAGER_H
