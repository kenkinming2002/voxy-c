#include "manager.h"
#include "coordinates.h"

#include "block/registry.h"
#include "light/manager.h"

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

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_init(&chunk_manager->active_chunks);
  chunk_hash_table_init(&chunk_manager->chunks);
}

void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager)
{
  chunk_hash_table_dispose(&chunk_manager->chunks);
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

uint8_t voxy_chunk_manager_get_block_id(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? chunk_get_block_id(chunk, global_position_to_local_position_i(position)) : def;
}

uint8_t voxy_chunk_manager_get_block_light_level(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? chunk_get_block_light_level(chunk, global_position_to_local_position_i(position)) : def;
}

void voxy_chunk_manager_set_block_id(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t id)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
    chunk_set_block_id(chunk, global_position_to_local_position_i(position), id);
}

void voxy_chunk_manager_set_block_light_level(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t light_level)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
    chunk_set_block_light_level(chunk, global_position_to_local_position_i(position), light_level);
}

bool voxy_chunk_manager_get_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
  {
    chunk_get_block_light_level_atomic(chunk, global_position_to_local_position_i(position), light_level, tmp);
    return true;
  }
  else
    return false;
}

bool voxy_chunk_manager_set_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? chunk_set_block_light_level_atomic(chunk, global_position_to_local_position_i(position), light_level, tmp) : false;
}

void voxy_chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager)
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

void voxy_chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct chunk_generator *chunk_generator, struct voxy_block_registry *block_registry, struct light_manager *light_manager, libnet_server_t server)
{
  // Generate and synchronize active chunks.
  {
    size_t generate_count = 0;
    size_t synchronize_count = 0;

    struct ivec3_node *position;
    SC_HASH_TABLE_FOREACH(chunk_manager->active_chunks, position)
    {
      struct chunk *chunk;
      if(!(chunk = chunk_hash_table_lookup(&chunk_manager->chunks, position->value)))
      {
        chunk = chunk_generator_generate(chunk_generator, position->value, block_registry);
        chunk_hash_table_insert_unchecked(&chunk_manager->chunks, chunk);
        for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
          for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
            for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
            {
              const ivec3_t local_position = ivec3(x, y, z);
              const ivec3_t global_position = local_position_to_global_position_i(local_position, chunk->position);
              if(chunk_get_block_light_level(chunk, local_position) != 0)
                light_manager_enqueue_creation_update(light_manager, global_position);
              else if(z == 0 || z == VOXY_CHUNK_WIDTH - 1 || y == 0 || y == VOXY_CHUNK_WIDTH - 1 || x == 0 || x == VOXY_CHUNK_WIDTH - 1)
                light_manager_enqueue_destruction_update(light_manager, global_position, 0);
            }

        generate_count += 1;
      }

      if(chunk->dirty)
      {
        struct voxy_server_chunk_update_message message;
        message.message.message.size = LIBNET_MESSAGE_SIZE(message);
        message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
        message.position = chunk->position;
        memcpy(&message.block_ids, &chunk->block_ids, sizeof chunk->block_ids);
        memcpy(&message.block_light_levels, &chunk->block_light_levels, sizeof chunk->block_light_levels);
        libnet_server_send_message_all(server, &message.message.message);

        chunk->dirty = false;
        synchronize_count += 1;
      }
    }

    if(generate_count != 0) LOG_INFO("Generated %zu chunks", generate_count);
    if(synchronize_count != 0) LOG_INFO("Synchronized %zu chunks over the network", synchronize_count);
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

void voxy_chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
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
