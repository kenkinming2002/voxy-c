#include "manager.h"
#include "player.h"

#include <voxy/protocol/server.h>
#include <voxy/protocol/client.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

void voxy_player_manager_init(struct voxy_player_manager *player_manager)
{
  player_manager->on_new_player = NULL;
}

void voxy_player_manager_fini(struct voxy_player_manager *player_manager)
{
  (void)player_manager;
}

void voxy_player_manager_set_on_new_player(struct voxy_player_manager *player_manager, voxy_on_new_player on_new_player)
{
  player_manager->on_new_player = on_new_player;
}

void voxy_player_manager_on_client_connected(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy, const struct voxy_context *context)
{
  struct voxy_player *player = voxy_player_create(server, client_proxy);
  libnet_client_proxy_set_opaque(client_proxy, player);
  if(player_manager->on_new_player)
    player_manager->on_new_player(player, context);
}

void voxy_player_manager_on_client_disconnected(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = libnet_client_proxy_get_opaque(client_proxy);
  voxy_player_put(player);
}

void voxy_player_manager_on_message_received(struct voxy_player_manager *player_manager, libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *_message)
{
  const struct voxy_client_input_message *message = voxy_get_client_input_message(_message);
  if(message)
  {
    struct voxy_player *player = libnet_client_proxy_get_opaque(client_proxy);

    player->left = message->left;
    player->right = message->right;
    player->back = message->back;
    player->front = message->front;
    player->bottom = message->bottom;
    player->top = message->top;

    player->mouse_motion = fvec2_add(player->mouse_motion, message->motion);

  }
}

