#ifndef PLAYER_MANAGER_H
#define PLAYER_MANAGER_H

#include <voxy/server/player/manager.h>

#include <libnet/server.h>

struct voxy_player_manager
{
  voxy_on_new_player on_new_player;
};

void voxy_player_manager_init(struct voxy_player_manager *player_manager);
void voxy_player_manager_fini(struct voxy_player_manager *player_manager);

void voxy_player_manager_on_client_connected(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy, const struct voxy_context *context);
void voxy_player_manager_on_client_disconnected(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);
void voxy_player_manager_on_message_received(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *_message);

#endif // PLAYER_MANAGER_H
