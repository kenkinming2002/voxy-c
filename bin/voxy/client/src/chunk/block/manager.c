#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>

#include <stb_ds.h>

#include <string.h>

struct block_group_node
{
  ivec3_t key;
  struct block_group *value;
};


static struct block_group_node *block_group_nodes;

struct block_group *get_block_group(ivec3_t chunk_position)
{
  ptrdiff_t i = hmgeti(block_group_nodes, chunk_position);
  return i != -1 ? block_group_nodes[i].value : NULL;
}

struct block_group *get_or_insert_block_group(ivec3_t chunk_position)
{
  struct block_group *block_group = get_block_group(chunk_position);
  if(!block_group)
  {
    block_group = block_group_create();
    for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    {
      const ivec3_t neighbour_position = ivec3_add(chunk_position, direction_as_ivec(direction));
      struct block_group *neighbour_block_group = get_block_group(neighbour_position);
      block_group->neighbours[direction] = neighbour_block_group;
      if(neighbour_block_group)
        neighbour_block_group->neighbours[direction_reverse(direction)] = block_group;
    }

    hmput(block_group_nodes, chunk_position, block_group);
  }
  return block_group;
}

void block_manager_remove_block_group(ivec3_t chunk_position)
{
  hmdel(block_group_nodes, chunk_position);
}

void block_manager_on_message_received(const struct libnet_message *_message)
{
  {
    const struct voxy_server_block_group_update_message *message = voxy_get_server_block_group_update_message(_message);
    if(message)
    {
      struct block_group *block_group = get_or_insert_block_group(message->position);

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
      block_manager_remove_block_group(message->position);
  }
}

