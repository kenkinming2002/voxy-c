#include "network.h"

#include <voxy/protocol/server.h>

#include <stdlib.h>
#include <string.h>

void voxy_block_network_update(ivec3_t position, const struct voxy_block_group *block_group)
{
  struct voxy_server_block_group_update_message *message = calloc(1, sizeof *message);
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
  message->message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
  message->position = position;

  memcpy(&message->block_ids, &block_group->ids, sizeof message->block_ids);
  memcpy(&message->block_lights, &block_group->lights, sizeof message->block_lights);

  libnet_server_send_message(NULL, &message->message.message);
}

void voxy_block_network_remove(ivec3_t position)
{
  struct voxy_server_block_group_remove_message *message = calloc(1, sizeof *message);
  message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
  message->message.tag = VOXY_SERVER_MESSAGE_CHUNK_REMOVE;
  message->position = position;
  libnet_server_send_message(NULL, &message->message.message);
}

