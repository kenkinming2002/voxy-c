#include "manager.h"

#include <voxy/protocol/protocol.h>
#include <string.h>

int chunk_manager_init(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
  return 0;
}

void chunk_manager_fini(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
}

void chunk_manager_update(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
}

void chunk_manager_on_client_connected(struct chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  (void)chunk_manager;

  const int radius = 10;
  for(int z=-radius; z<=radius; ++z)
    for(int y=-radius; y<=radius; ++y)
      for(int x=-radius; x<=radius; ++x)
      {
        struct voxy_chunk_update_message message;

        message.message.message.size = LIBNET_MESSAGE_SIZE(message);
        message.message.tag = VOXY_MESSAGE_CHUNK_UPDATE;

        message.position = ivec3(x, y, z);

        for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
          for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
            for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
              message.block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = x < 8 && y < 8 && z < 8;

        memset(message.block_light_levels, 0xFF, sizeof message.block_light_levels);

        libnet_server_send_message(server, client_proxy, &message.message.message);
      }
}
