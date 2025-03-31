#include "network.h"

#include <voxy/protocol/server.h>
#include <string.h>

void voxy_block_network_update(ivec3_t position, const struct voxy_block_group *block_group, libnet_server_t server)
{
  struct voxy_server_block_group_update_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
  message.position = position;
  memcpy(&message.block_ids, &block_group->ids, sizeof block_group->ids);
  memcpy(&message.block_light_levels, &block_group->light_levels, sizeof block_group->light_levels);
  libnet_server_send_message_all(server, &message.message.message);
}

void voxy_block_network_remove(ivec3_t position, libnet_server_t server)
{
  struct voxy_server_block_group_remove_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_REMOVE;
  message.position = position;
  libnet_server_send_message_all(server, &message.message.message);
}

