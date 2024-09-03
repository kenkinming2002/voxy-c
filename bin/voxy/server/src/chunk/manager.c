#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcommon/core/log.h>

#include <string.h>
#include <stdlib.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX ivec3
#define SC_HASH_TABLE_NODE_TYPE struct ivec3_node
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t ivec3_key(struct ivec3_node *node) { return node->value; }
void ivec3_dispose(struct ivec3_node *node) { free(node); }

static void chunk_init(struct chunk *chunk, ivec3_t position)
{
  chunk->position = position;

  for(int z=0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y=0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x=0; x<VOXY_CHUNK_WIDTH; ++x)
        chunk->block_ids[z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + y * VOXY_CHUNK_WIDTH + x] = (x < 8 && y < 8 && z < 8) ? (x + y + z) % 4 : 0;

  memset(&chunk->block_light_levels, 0xFF, sizeof chunk->block_light_levels);
}

void chunk_manager_init(struct chunk_manager *chunk_manager)
{
  ivec3_hash_table_init(&chunk_manager->active_chunks);
  chunk_hash_table_init(&chunk_manager->chunks);

  const int radius = 10;
  for(int z=-radius; z<=radius; ++z)
    for(int y=-radius; y<=radius; ++y)
      for(int x=-radius; x<=radius; ++x)
      {
        struct ivec3_node *position = malloc(sizeof *position);
        position->value = ivec3(x, y, z);
        ivec3_hash_table_insert_unchecked(&chunk_manager->active_chunks, position);
      }
}

void chunk_manager_fini(struct chunk_manager *chunk_manager)
{
  chunk_hash_table_dispose(&chunk_manager->chunks);
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

void chunk_manager_update(struct chunk_manager *chunk_manager, libnet_server_t server)
{
  // Generate active chunks.
  {
    struct ivec3_node *position;
    struct chunk *chunk;
    SC_HASH_TABLE_FOREACH(chunk_manager->active_chunks, position)
      if(!(chunk = chunk_hash_table_lookup(&chunk_manager->chunks, position->value)))
      {
        chunk = chunk_create();
        chunk_init(chunk, position->value);
        chunk_hash_table_insert_unchecked(&chunk_manager->chunks, chunk);

        struct voxy_server_chunk_update_message message;
        message.message.message.size = LIBNET_MESSAGE_SIZE(message);
        message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
        message.position = chunk->position;
        memcpy(&message.block_ids, &chunk->block_ids, sizeof chunk->block_ids);
        memcpy(&message.block_light_levels, &chunk->block_light_levels, sizeof chunk->block_light_levels);
        libnet_server_send_message_all(server, &message.message.message);
      }
  }

  // Discard non-active chunks.
  {
    for(size_t i=0; i<chunk_manager->chunks.bucket_count; ++i)
    {
      struct chunk **chunk = &chunk_manager->chunks.buckets[i].head;
      while(*chunk)
        if(!ivec3_hash_table_lookup(&chunk_manager->active_chunks, (*chunk)->position))
        {
          struct chunk *old_chunk = *chunk;
          *chunk = (*chunk)->next;

          struct voxy_server_chunk_update_message message;
          message.message.message.size = LIBNET_MESSAGE_SIZE(message);
          message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_REMOVE;
          message.position = old_chunk->position;
          libnet_server_send_message_all(server, &message.message.message);

          chunk_destroy(old_chunk);
        }
        else
          chunk = &(*chunk)->next;
    }
  }
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
