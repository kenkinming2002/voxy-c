#include "manager.h"
#include "player.h"

#include <voxy/protocol/server.h>
#include <voxy/protocol/client.h>

#include <libmath/matrix.h>
#include <libmath/matrix_transform.h>

#include <string.h>
#include <stdio.h>

static voxy_on_new_player on_new_player;

void voxy_set_on_new_player(voxy_on_new_player _on_new_player)
{
  on_new_player = _on_new_player;
}

void voxy_player_manager_on_client_connected(libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = voxy_player_create(client_proxy);
  libnet_client_proxy_set_opaque(client_proxy, player);
}

void voxy_player_manager_on_client_disconnected(libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = libnet_client_proxy_get_opaque(client_proxy);
  voxy_player_put(player);
}

void voxy_player_manager_on_message_received(libnet_client_proxy_t client_proxy, const struct libnet_message *_message, const struct voxy_context *context)
{
  {
    const struct voxy_client_login_message *message = voxy_get_client_login_message(_message);
    if(message)
    {
      struct voxy_player *player = libnet_client_proxy_get_opaque(client_proxy);
      player->name = strndup(message->player_name, message->message.message.size + sizeof(struct libnet_message) - offsetof(struct voxy_client_login_message, player_name));

      printf("Player %s logs in\n", player->name);
      if(on_new_player)
        on_new_player(player, context);

    }
  }
  {
    const struct voxy_client_input_message *message = voxy_get_client_input_message(_message);
    if(message)
    {
      struct voxy_player *player = libnet_client_proxy_get_opaque(client_proxy);

      player->key_left = message->key_left;
      player->key_right = message->key_right;
      player->key_back = message->key_back;
      player->key_front = message->key_front;
      player->key_bottom = message->key_bottom;
      player->key_top = message->key_top;

      player->mouse_button_left = message->mouse_button_left;
      player->mouse_button_right = message->mouse_button_right;

      player->mouse_motion = fvec2_add(player->mouse_motion, message->mouse_motion);

    }
  }
}

