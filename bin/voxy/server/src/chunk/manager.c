#include "manager.h"
#include "coordinates.h"

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

void chunk_manager_init(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_init(&chunk_manager->active_chunks);
  chunk_hash_table_init(&chunk_manager->chunks);
}

void chunk_manager_fini(struct voxy_chunk_manager *chunk_manager)
{
  chunk_hash_table_dispose(&chunk_manager->chunks);
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

uint8_t chunk_manager_get_block_id(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? chunk_get_block_id(chunk, global_position_to_local_position_i(position)) : def;
}

uint8_t chunk_manager_get_block_light_level(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? chunk_get_block_light_level(chunk, global_position_to_local_position_i(position)) : def;
}

void chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

void voxy_chunk_manager_add_active_chunk(struct voxy_chunk_manager *chunk_manager, ivec3_t position)
{
  struct ivec3_node *node;
  if(!(node = ivec3_hash_table_lookup(&chunk_manager->active_chunks, position)))
  {
    node = malloc(sizeof *node);
    node->value = position;
    ivec3_hash_table_insert_unchecked(&chunk_manager->active_chunks, node);
  }
}

void chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct chunk_generator *chunk_generator, libnet_server_t server)
{
  // Generate active chunks.
  {
    struct ivec3_node *position;
    struct chunk *chunk;
    SC_HASH_TABLE_FOREACH(chunk_manager->active_chunks, position)
      if(!(chunk = chunk_hash_table_lookup(&chunk_manager->chunks, position->value)))
      {
        chunk = chunk_generator_generate(chunk_generator, position->value);
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

void chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
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
