#include "network.h"

#include <stdlib.h>

#include <voxy/protocol/server.h>

void voxy_entity_network_update(entity_handle_t handle, const struct voxy_entity *entity, libnet_client_proxy_t proxy)
{
  struct voxy_server_entity_update_message *message = calloc(1, sizeof *message);
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
  message->message.tag = VOXY_SERVER_MESSAGE_ENTITY_UPDATE;
  message->handle = handle;
  message->id = entity->id;
  message->position = entity->position;
  message->rotation = entity->rotation;
  libnet_server_send_message(proxy, &message->message.message);
}

void voxy_entity_network_update_all(entity_handle_t handle, const struct voxy_entity *entity)
{
  struct voxy_server_entity_update_message *message = calloc(1, sizeof *message);
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
  message->message.tag = VOXY_SERVER_MESSAGE_ENTITY_UPDATE;
  message->handle = handle;
  message->id = entity->id;
  message->position = entity->position;
  message->rotation = entity->rotation;
  libnet_server_send_message(NULL, &message->message.message);
}

void voxy_entity_network_remove_all(entity_handle_t handle)
{
  struct voxy_server_entity_remove_message *message = calloc(1, sizeof *message);
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
  message->message.tag = VOXY_SERVER_MESSAGE_ENTITY_REMOVE;
  message->handle = handle;
  libnet_server_send_message(NULL, &message->message.message);
}
