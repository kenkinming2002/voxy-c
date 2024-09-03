#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcommon/core/log.h>

#include <string.h>

int chunk_manager_init(struct chunk_manager *chunk_manager)
{
  chunk_hash_table_init(&chunk_manager->chunks);
  return 0;
}

void chunk_manager_fini(struct chunk_manager *chunk_manager)
{
  chunk_hash_table_dispose(&chunk_manager->chunks);
}

struct chunk *chunk_manager_get_chunk(struct chunk_manager *chunk_manager, ivec3_t position)
{
  struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, position);
  if(!chunk)
  {
    chunk = chunk_create();
    chunk->position = position;
    chunk_hash_table_insert_unchecked(&chunk_manager->chunks, chunk);

    for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    {
      const ivec3_t neighbour_position = ivec3_add(chunk->position, direction_as_ivec(direction));
      struct chunk *neighbour_chunk = chunk_hash_table_lookup(&chunk_manager->chunks, neighbour_position);

      chunk->neighbours[direction] = neighbour_chunk;
      if(neighbour_chunk)
        neighbour_chunk->neighbours[direction_reverse(direction)] = chunk;
    }
  }
  return chunk;
}

void chunk_manager_remove_chunk(struct chunk_manager *chunk_manager, ivec3_t position)
{
  chunk_hash_table_remove(&chunk_manager->chunks, position);
}

void chunk_manager_update(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
}

void chunk_manager_on_message_received(struct chunk_manager *chunk_manager, libnet_client_t client, const struct libnet_message *_message)
{
  {
    const struct voxy_server_chunk_update_message *message = voxy_get_server_chunk_update_message(_message);
    if(message)
    {
      struct chunk *chunk = chunk_manager_get_chunk(chunk_manager, message->position);
      memcpy(&chunk->block_ids, &message->block_ids, sizeof message->block_ids);
      memcpy(&chunk->block_light_levels, &message->block_light_levels, sizeof message->block_light_levels);
    }
  }

  {
    const struct voxy_server_chunk_remove_message *message = voxy_get_server_chunk_remove_message(_message);
    if(message)
      chunk_manager_remove_chunk(chunk_manager, message->position);
  }
}

