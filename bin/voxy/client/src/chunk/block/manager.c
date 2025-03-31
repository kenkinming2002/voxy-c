#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>

#include <stb_ds.h>

#include <string.h>

int block_manager_init(struct block_manager *block_manager)
{
  block_manager->block_group_nodes = NULL;
  return 0;
}

void block_manager_fini(struct block_manager *block_manager)
{
  hmfree(block_manager->block_group_nodes);
}

static struct block_group *block_group_lookup(struct block_manager *block_manager, ivec3_t position)
{
  ptrdiff_t i = hmgeti(block_manager->block_group_nodes, position);
  return i != -1 ? block_manager->block_group_nodes[i].value : NULL;
}

struct block_group *block_manager_get_block_group(struct block_manager *block_manager, ivec3_t position)
{
  struct block_group *block_group = block_group_lookup(block_manager, position);
  if(!block_group)
  {
    block_group = block_group_create();
    for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    {
      const ivec3_t neighbour_position = ivec3_add(position, direction_as_ivec(direction));
      struct block_group *neighbour_block_group = block_group_lookup(block_manager, neighbour_position);
      block_group->neighbours[direction] = neighbour_block_group;
      if(neighbour_block_group)
        neighbour_block_group->neighbours[direction_reverse(direction)] = block_group;
    }

    hmput(block_manager->block_group_nodes, position, block_group);
  }
  return block_group;
}

void block_manager_remove_block_group(struct block_manager *block_manager, ivec3_t position)
{
  hmdel(block_manager->block_group_nodes, position);
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

