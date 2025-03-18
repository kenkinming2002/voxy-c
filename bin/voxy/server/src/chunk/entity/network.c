#include "network.h"

#include <voxy/protocol/server.h>

void voxy_entity_network_update(entity_handle_t handle, const struct voxy_entity *entity, libnet_server_t server, libnet_client_proxy_t proxy)
{
  struct voxy_server_entity_update_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_ENTITY_UPDATE;
  message.handle = handle;
  message.id = entity->id;
  message.position = entity->position;
  message.rotation = entity->rotation;
  libnet_server_send_message(server, proxy, &message.message.message);
}

void voxy_entity_network_update_all(entity_handle_t handle, const struct voxy_entity *entity, libnet_server_t server)
{
  struct voxy_server_entity_update_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_ENTITY_UPDATE;
  message.handle = handle;
  message.id = entity->id;
  message.position = entity->position;
  message.rotation = entity->rotation;
  libnet_server_send_message_all(server, &message.message.message);
}

void voxy_entity_network_remove_all(entity_handle_t handle, libnet_server_t server)
{
  struct voxy_server_entity_remove_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_ENTITY_REMOVE;
  message.handle = handle;
  libnet_server_send_message_all(server, &message.message.message);
}
