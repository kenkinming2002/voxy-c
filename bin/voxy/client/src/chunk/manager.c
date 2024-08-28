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

void chunk_manager_update(struct chunk_manager *chunk_manager)
{
  (void)chunk_manager;
}

void chunk_manager_on_message_received(struct chunk_manager *chunk_manager, libnet_client_t client, const struct libnet_message *message)
{
  const struct voxy_server_chunk_update_message *_message = voxy_get_server_chunk_update_message(message);
  if(!_message)
    return;

  const ivec3_t position = _message->position;
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

  memcpy(&chunk->block_ids, &_message->block_ids, sizeof _message->block_ids);
  memcpy(&chunk->block_light_levels, &_message->block_light_levels, sizeof _message->block_light_levels);
}

