#include "manager.h"
#include "player.h"

#include <voxy/protocol/server.h>
#include <voxy/protocol/client.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

struct player_manager_update_callback_context
{
  float dt;
  struct voxy_entity_manager *entity_manager;
};

static void player_manager_update_callback(libnet_server_t server, libnet_client_proxy_t client_proxy, void *data)
{
  struct player_manager_update_callback_context *context = data;
  struct player *player = libnet_client_proxy_get_opaque(client_proxy);
  player_update(player, context->dt, context->entity_manager);
}

void player_manager_update(float dt, libnet_server_t server, struct voxy_entity_manager *entity_manager)
{
  struct player_manager_update_callback_context context;
  context.dt = dt;
  context.entity_manager = entity_manager;
  libnet_server_foreach_client(server, player_manager_update_callback, &context);
}

void player_manager_on_client_connected(libnet_server_t server, libnet_client_proxy_t client_proxy, struct voxy_entity_manager *entity_manager)
{
  struct player *player = player_create(entity_manager, server, client_proxy);
  libnet_client_proxy_set_opaque(client_proxy, player);
}

void player_manager_on_client_disconnected(libnet_server_t server, libnet_client_proxy_t client_proxy, struct voxy_entity_manager *entity_manager)
{
  struct player *player = libnet_client_proxy_get_opaque(client_proxy);
  player_destroy(player, entity_manager, server);
}

void player_manager_on_message_received(libnet_server_t server, libnet_client_proxy_t client_proxy, const struct libnet_message *_message)
{
  const struct voxy_client_input_message *message = voxy_get_client_input_message(_message);
  if(message)
  {
    struct player *player = libnet_client_proxy_get_opaque(client_proxy);

    player->left = message->left;
    player->right = message->right;
    player->back = message->back;
    player->front = message->front;
    player->bottom = message->bottom;
    player->top = message->top;

    player->mouse_motion = fvec2_add(player->mouse_motion, message->motion);
  }
}

