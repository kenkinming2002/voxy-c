#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>

#include <string.h>

int block_manager_init(struct block_manager *block_manager)
{
  block_group_hash_table_init(&block_manager->block_groups);
  return 0;
}

void block_manager_fini(struct block_manager *block_manager)
{
  block_group_hash_table_dispose(&block_manager->block_groups);
}

struct block_group *block_manager_get_block_group(struct block_manager *block_manager, ivec3_t position)
{
  struct block_group *block_group = block_group_hash_table_lookup(&block_manager->block_groups, position);
  if(!block_group)
  {
    block_group = block_group_create();
    block_group->position = position;
    block_group_hash_table_insert_unchecked(&block_manager->block_groups, block_group);

    for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    {
      const ivec3_t neighbour_position = ivec3_add(block_group->position, direction_as_ivec(direction));
      struct block_group *neighbour_block_group = block_group_hash_table_lookup(&block_manager->block_groups, neighbour_position);

      block_group->neighbours[direction] = neighbour_block_group;
      if(neighbour_block_group)
        neighbour_block_group->neighbours[direction_reverse(direction)] = block_group;
    }
  }
  return block_group;
}

void block_manager_remove_block_group(struct block_manager *block_manager, ivec3_t position)
{
  block_group_hash_table_remove(&block_manager->block_groups, position);
}

void block_manager_update(struct block_manager *block_manager)
{
  (void)block_manager;
}

void block_manager_on_message_received(struct block_manager *block_manager, libnet_client_t client, const struct libnet_message *_message)
{
  {
    const struct voxy_server_block_group_update_message *message = voxy_get_server_block_group_update_message(_message);
    if(message)
    {
      struct block_group *block_group = block_manager_get_block_group(block_manager, message->position);

      memcpy(&block_group->block_ids, &message->block_ids, sizeof message->block_ids);
      memcpy(&block_group->block_light_levels, &message->block_light_levels, sizeof message->block_light_levels);

      block_group->remesh = true;
      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if(block_group->neighbours[direction])
          block_group->neighbours[direction]->remesh = true;
    }
  }

  {
    const struct voxy_server_block_group_remove_message *message = voxy_get_server_block_group_remove_message(_message);
    if(message)
      block_manager_remove_block_group(block_manager, message->position);
  }
}

