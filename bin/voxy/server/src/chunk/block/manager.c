#include "manager.h"

#include <voxy/server/registry/block.h>

#include "chunk/block/generator.h"
#include "chunk/coordinates.h"
#include "chunk/manager.h"

#include "light/light.h"

#include "database.h"
#include "network.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <string.h>
#include <stdlib.h>

struct block_group_node
{
  ivec3_t key;
  struct voxy_block_group *value;
};

static struct block_group_node *block_group_nodes;

struct voxy_block_group *voxy_get_block_group(ivec3_t chunk_position)
{
  struct block_group_node *block_group_node = hmgetp_null(block_group_nodes, chunk_position);
  return block_group_node ? block_group_node->value : NULL;
}

uint8_t voxy_get_block_id(ivec3_t position, uint8_t def)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return def;

  return voxy_block_group_get_id(block_group, global_position_to_local_position_i(position));
}

voxy_light_t voxy_get_block_light_level(ivec3_t position, voxy_light_t def)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return def;

  return voxy_block_group_get_light(block_group, global_position_to_local_position_i(position));
}

bool voxy_set_block(ivec3_t position, voxy_block_id_t id)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return false;

  const voxy_light_t old_light = voxy_block_group_get_light(block_group, global_position_to_local_position_i(position));
  const uint8_t light_level = voxy_query_block(id).light_level;

  voxy_block_group_set_id(block_group, global_position_to_local_position_i(position), id);
  voxy_block_group_set_light(block_group, global_position_to_local_position_i(position), (voxy_light_t){ .level = voxy_query_block(id).light_level, .sol = 0});

  if(old_light.level < light_level)
    enqueue_light_creation_update(position);

  //// FIXME: Light destroy update really only support the case if light level
  ////        changes to 0. This happens to be the only possible cases for now.
  if(old_light.level >= light_level)
    enqueue_light_destruction_update(position, old_light);

  return true;
}

bool voxy_set_block_id(ivec3_t position, voxy_block_id_t id)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return false;

  voxy_block_group_set_id(block_group, global_position_to_local_position_i(position), id);
  return true;
}

bool voxy_set_block_light(ivec3_t position, voxy_light_t light)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return false;

  voxy_block_group_set_light(block_group, global_position_to_local_position_i(position), light);
  return true;
}

struct major_chunk_statistic
{
  size_t generate_count;
  size_t load_count;
};

static void block_manager_major_chunk_iterate(ivec3_t chunk_position, void *data)
{
  struct major_chunk_statistic *statistic = data;

  if(hmgetp_null(block_group_nodes, chunk_position))
    return;

  struct voxy_block_group *block_group = NULL;

  if(!block_group)
  {
    struct block_group_future future = voxy_block_database_load(chunk_position);
    if(future.pending)
      return;

    block_group = future.value;
    if(block_group)
      statistic->load_count += 1;
  }

  if(!block_group)
  {
    struct block_group_future future = voxy_block_group_generate(chunk_position);
    if(future.pending)
      return;

    block_group = future.value;
    if(block_group)
      statistic->generate_count += 1;
  }

  if(!block_group)
    return;

  hmput(block_group_nodes, chunk_position, block_group);
  for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
    if((block_group->neighbours[direction] = voxy_get_block_group(ivec3_add(chunk_position, direction_as_ivec(direction)))))
      block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = block_group;

  for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const ivec3_t local_position = ivec3(x, y, z);
        const ivec3_t global_position = local_position_to_global_position_i(local_position, chunk_position);
        if(voxy_block_group_get_light(block_group, local_position).level != 0)
          enqueue_light_creation_update(global_position);
        else if(z == 0 || z == VOXY_CHUNK_WIDTH - 1 || y == 0 || y == VOXY_CHUNK_WIDTH - 1 || x == 0 || x == VOXY_CHUNK_WIDTH - 1)
          enqueue_light_destruction_update(global_position, (voxy_light_t){ .level = 0, .sol = 0, });
      }
}

void voxy_block_manager_update(void)
{
  profile_scope;

  // Iterate major chunk and try to load/generate block groups
  {
    profile_scope;

    struct major_chunk_statistic statistic = {0};
    iterate_major_chunk(&block_manager_major_chunk_iterate, &statistic);
  }

  // Iterate existing block groups and try to flush them
  {
    profile_scope;

    size_t save_count = 0;
    size_t send_count = 0;

    for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
    {
      ivec3_t position = block_group_nodes[i].key;
      struct voxy_block_group *block_group = block_group_nodes[i].value;

      if(block_group->disk_dirty && !voxy_block_database_save(position, block_group).pending)
      {
        block_group->disk_dirty = false;
        save_count += 1;
      }

      if(block_group->network_dirty)
      {
        voxy_block_network_update(position, block_group);
        block_group->network_dirty = false;
        send_count += 1;
      }
    }

    if(save_count != 0) LOG_INFO("Block Manager: Saved %zu block groups to disk", save_count);
    if(send_count != 0) LOG_INFO("Block Manager: Sent %zu block groups over the network", send_count);
  }

  // Iterate existing block groups and try to unload them
  {
    profile_scope;

    size_t discard_count = 0;

    struct block_group_node *new_block_group_nodes = NULL;
    for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
    {
      ivec3_t chunk_position = block_group_nodes[i].key;
      struct voxy_block_group *block_group = block_group_nodes[i].value;

      if(block_group->disk_dirty)
      {
        hmput(new_block_group_nodes, chunk_position, block_group);
        continue;
      }

      if(is_minor_chunk(chunk_position))
      {
        hmput(new_block_group_nodes, chunk_position, block_group);
        continue;
      }

      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if(block_group->neighbours[direction])
          block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = NULL;

      voxy_block_network_remove(chunk_position);
      voxy_block_group_destroy(block_group);
      discard_count += 1;
    }

    hmfree(block_group_nodes);
    block_group_nodes = new_block_group_nodes;

    if(discard_count != 0) LOG_INFO("Block Manager: Discarded %zu block groups", discard_count);
  }
}

void voxy_block_manager_on_client_connected(libnet_client_proxy_t client_proxy)
{
  for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
  {
    ivec3_t position = block_group_nodes[i].key;
    struct voxy_block_group *block_group = block_group_nodes[i].value;

    struct voxy_server_block_group_update_message *message = calloc(1, sizeof *message);
    message->message.message.size = LIBNET_MESSAGE_SIZE(*message);
    message->message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
    message->position = position;

    memcpy(&message->block_ids, &block_group->ids, sizeof message->block_ids);
    memcpy(&message->block_lights, &block_group->lights, sizeof message->block_lights);

    libnet_server_send_message(client_proxy, &message->message.message);
  }
}
