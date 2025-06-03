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
  ptrdiff_t i = hmgeti(block_group_nodes, chunk_position);
  return i != -1 ? block_group_nodes[i].value : NULL;
}

uint8_t voxy_get_block_id(ivec3_t position, uint8_t def)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  return block_group ? voxy_block_group_get_block_id(block_group, global_position_to_local_position_i(position)) : def;
}

uint8_t voxy_get_block_light_level(ivec3_t position, uint8_t def)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  return block_group ? voxy_block_group_get_block_light_level(block_group, global_position_to_local_position_i(position)) : def;
}

void voxy_set_block_id(ivec3_t position, uint8_t id)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(block_group)
    voxy_block_group_set_block_id(block_group, global_position_to_local_position_i(position), id);
}

void voxy_set_block_light_level(ivec3_t position, uint8_t light_level)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(block_group)
    voxy_block_group_set_block_light_level(block_group, global_position_to_local_position_i(position), light_level);
}

/// Set block_group at given position.
///
/// The light level of the block_group will be derived from light info in block_group
/// registry. This will also enqueue any necessary light updates to light
/// manager..
VOXY_SERVER_EXPORT void voxy_set_block(ivec3_t position, uint8_t id)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(!block_group)
    return;

  const uint8_t old_light_level = voxy_block_group_get_block_light_level(block_group, global_position_to_local_position_i(position));
  const uint8_t light_level = voxy_query_block(id).light_level;

  voxy_block_group_set_block_id(block_group, global_position_to_local_position_i(position), id);
  voxy_block_group_set_block_light_level(block_group, global_position_to_local_position_i(position), light_level);

  if(old_light_level < light_level)
    enqueue_light_creation_update(position);

  //// FIXME: Light destroy update really only support the case if light level
  ////        changes to 0. This happens to be the only possible cases for now.
  if(old_light_level >= light_level)
    enqueue_light_destruction_update(position, old_light_level);
}

bool voxy_block_manager_get_block_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  if(block_group)
  {
    voxy_block_group_get_block_light_level_atomic(block_group, global_position_to_local_position_i(position), light_level, tmp);
    return true;
  }
  else
    return false;
}

bool voxy_block_manager_set_block_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_block_group *block_group = voxy_get_block_group(get_chunk_position_i(position));
  return block_group ? voxy_block_group_set_block_light_level_atomic(block_group, global_position_to_local_position_i(position), light_level, tmp) : false;
}

static struct block_group_future load_or_generate_block(ivec3_t position, size_t *load_count, size_t *generate_count)
{
  profile_scope;

  struct block_group_future result;

  result = voxy_block_database_load(position);
  if(result.value || result.pending)
  {
    *load_count += !result.pending;
    return result;
  }

  result = voxy_block_group_generate(position);
  if(result.value || result.pending)
  {
    *generate_count += !result.pending;
    return result;
  }

  return block_group_future_ready(NULL);
}

static void load_or_generate_blocks(void)
{
  profile_begin();

  size_t load_count = 0;
  size_t generate_count = 0;

  for(ptrdiff_t i=0; i<hmlen(active_chunks); ++i)
  {
    ivec3_t position = active_chunks[i].key;
    if(hmgeti(block_group_nodes, position) != -1)
      continue;

    struct block_group_future block_group_future = load_or_generate_block(position, &load_count, &generate_count);
    struct voxy_block_group *block_group;
    if((block_group = block_group_future.value))
    {
      hmput(block_group_nodes, position, block_group);

      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if((block_group->neighbours[direction] = voxy_get_block_group(ivec3_add(position, direction_as_ivec(direction)))))
          block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = block_group;

      for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
        for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
          for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
          {
            const ivec3_t local_position = ivec3(x, y, z);
            const ivec3_t global_position = local_position_to_global_position_i(local_position, position);
            if(voxy_block_group_get_block_light_level(block_group, local_position) != 0)
              enqueue_light_creation_update(global_position);
            else if(z == 0 || z == VOXY_CHUNK_WIDTH - 1 || y == 0 || y == VOXY_CHUNK_WIDTH - 1 || x == 0 || x == VOXY_CHUNK_WIDTH - 1)
              enqueue_light_destruction_update(global_position, 0);
          }
    }
  }

  if(generate_count != 0) LOG_INFO("Block Manager: Generated %zu block groups", generate_count);
  if(load_count != 0) LOG_INFO("Block Manager: Loaded %zu block groups from disk", load_count);

  profile_end("load_count", tformat("%zu", load_count),
              "generate_count", tformat("%zu", generate_count),
              "total_count", tformat("%zu", load_count + generate_count));
}

static void flush_blocks(void)
{
  profile_begin();

  size_t save_count = 0;
  size_t send_count = 0;

  for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
  {
    ivec3_t position = block_group_nodes[i].key;
    struct voxy_block_group *block_group = block_group_nodes[i].value;

    if(block_group->disk_dirty)
    {
      struct unit_future result = voxy_block_database_save(position, block_group);
      if(!result.pending)
      {
        block_group->disk_dirty = false;
        save_count += 1;
      }
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

  profile_end("send_count", tformat("%zu", send_count),
              "save_count", tformat("%zu", save_count));
}

static void discard_blocks(void)
{
  profile_begin();

  size_t discard_count = 0;

  struct block_group_node *new_block_group_nodes = NULL;
  for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
  {
    ivec3_t position = block_group_nodes[i].key;
    struct voxy_block_group *block_group = block_group_nodes[i].value;

    if(hmgeti(active_chunks, position) == -1 && !block_group->disk_dirty)
    {
      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if(block_group->neighbours[direction])
          block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = NULL;

      voxy_block_network_remove(position);
      voxy_block_group_destroy(block_group);

      discard_count += 1;
    }
    else
      hmput(new_block_group_nodes, position, block_group);
  }

  hmfree(block_group_nodes);
  block_group_nodes = new_block_group_nodes;

  if(discard_count != 0) LOG_INFO("Block Manager: Discarded %zu block groups", discard_count);

  profile_end("discard_count", tformat("%zu", discard_count));
}

void voxy_block_manager_update(void)
{
  profile_scope;

  load_or_generate_blocks();
  flush_blocks();
  discard_blocks();
}

void voxy_block_manager_on_client_connected(libnet_client_proxy_t client_proxy)
{
  for(ptrdiff_t i=0; i<hmlen(block_group_nodes); ++i)
  {
    ivec3_t position = block_group_nodes[i].key;
    struct voxy_block_group *block_group = block_group_nodes[i].value;

    struct voxy_server_block_group_update_message message;
    message.message.message.size = LIBNET_MESSAGE_SIZE(message);
    message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
    message.position = position;
    memcpy(&message.block_ids, &block_group->ids, sizeof block_group->ids);
    memcpy(&message.block_light_levels, &block_group->light_levels, sizeof block_group->light_levels);
    libnet_server_send_message(client_proxy, &message.message.message);
  }
}
