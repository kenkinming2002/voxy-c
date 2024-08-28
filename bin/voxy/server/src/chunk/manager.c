#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcommon/core/log.h>

#include <string.h>

int chunk_manager_init(struct chunk_manager *chunk_manager)
{
  chunk_hash_table_init(&chunk_manager->chunks);

  const int radius = 10;
  for(int z=-radius; z<=radius; ++z)
    for(int y=-radius; y<=radius; ++y)
      for(int x=-radius; x<=radius; ++x)
      {
        struct chunk *chunk = chunk_create();
        chunk->position = ivec3(x, y, z);
        {
          for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
            for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
              for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
                chunk->block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = x < 8 && y < 8 && z < 8;

          memset(&chunk->block_light_levels, 0xFF, sizeof chunk->block_light_levels);
        }
        chunk_hash_table_insert_unchecked(&chunk_manager->chunks, chunk);
      }

  return 0;
}

void chunk_manager_fini(struct chunk_manager *chunk_manager)
{
  chunk_hash_table_dispose(&chunk_manager->chunks);
}

void chunk_manager_update(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
}

void chunk_manager_on_client_connected(struct chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct chunk *chunk;
  SC_HASH_TABLE_FOREACH(chunk_manager->chunks, chunk)
  {
    struct voxy_server_chunk_update_message message;
    message.message.message.size = LIBNET_MESSAGE_SIZE(message);
    message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
    message.position = chunk->position;
    memcpy(&message.block_ids, &chunk->block_ids, sizeof chunk->block_ids);
    memcpy(&message.block_light_levels, &chunk->block_light_levels, sizeof chunk->block_light_levels);
    libnet_server_send_message(server, client_proxy, &message.message.message);
  }
}
