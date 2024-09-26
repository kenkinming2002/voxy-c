#include "network.h"

#include <voxy/protocol/server.h>
#include <string.h>

void chunk_network_update(const struct chunk *chunk, libnet_server_t server)
{
  struct voxy_server_chunk_update_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
  message.position = chunk->position;
  memcpy(&message.block_ids, &chunk->block_ids, sizeof chunk->block_ids);
  memcpy(&message.block_light_levels, &chunk->block_light_levels, sizeof chunk->block_light_levels);
  libnet_server_send_message_all(server, &message.message.message);
}

void chunk_network_remove(const struct chunk *chunk, libnet_server_t server)
{
  struct voxy_server_chunk_remove_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_REMOVE;
  message.position = chunk->position;
  libnet_server_send_message_all(server, &message.message.message);
}

