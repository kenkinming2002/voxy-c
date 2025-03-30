#include "manager.h"

#include "chunk/coordinates.h"

#include "database.h"
#include "network.h"

#include "registry/block.h"
#include "light/manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <string.h>
#include <stdlib.h>

void voxy_block_manager_init(struct voxy_block_manager *block_manager)
{
  voxy_block_group_hash_table_init(&block_manager->block_groups);
}

void voxy_block_manager_fini(struct voxy_block_manager *block_manager)
{
  voxy_block_group_hash_table_dispose(&block_manager->block_groups);
}

uint8_t voxy_block_manager_get_block_id(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t def)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  return block_group ? voxy_block_group_get_block_id(block_group, global_position_to_local_position_i(position)) : def;
}

uint8_t voxy_block_manager_get_block_light_level(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t def)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  return block_group ? voxy_block_group_get_block_light_level(block_group, global_position_to_local_position_i(position)) : def;
}

void voxy_block_manager_set_block_id(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t id)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  if(block_group)
    voxy_block_group_set_block_id(block_group, global_position_to_local_position_i(position), id);
}

void voxy_block_manager_set_block_light_level(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t light_level)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  if(block_group)
    voxy_block_group_set_block_light_level(block_group, global_position_to_local_position_i(position), light_level);
}

/// Set block_group at given position.
///
/// The light level of the block_group will be derived from light info in block_group
/// registry. This will also enqueue any necessary light updates to light
/// manager..
VOXY_SERVER_EXPORT void voxy_block_manager_set_block(
    struct voxy_block_manager *block_manager,
    struct voxy_block_registry *block_registry,
    struct voxy_light_manager *light_manager,
    ivec3_t position,
    uint8_t id)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  if(!block_group)
    return;

  const uint8_t old_light_level = voxy_block_group_get_block_light_level(block_group, global_position_to_local_position_i(position));
  const uint8_t light_level = voxy_block_registry_query_block(block_registry, id).light_level;

  voxy_block_group_set_block_id(block_group, global_position_to_local_position_i(position), id);
  voxy_block_group_set_block_light_level(block_group, global_position_to_local_position_i(position), light_level);

  if(old_light_level < light_level)
    voxy_light_manager_enqueue_creation_update(light_manager, block_manager, position);

  //// FIXME: Light destroy update really only support the case if light level
  ////        changes to 0. This happens to be the only possible cases for now.
  if(old_light_level >= light_level)
    voxy_light_manager_enqueue_destruction_update(light_manager, block_manager, position, old_light_level);
}

bool voxy_block_manager_get_block_light_level_atomic(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  if(block_group)
  {
    voxy_block_group_get_block_light_level_atomic(block_group, global_position_to_local_position_i(position), light_level, tmp);
    return true;
  }
  else
    return false;
}

bool voxy_block_manager_set_block_light_level_atomic(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_block_group *block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, get_chunk_position_i(position));
  return block_group ? voxy_block_group_set_block_light_level_atomic(block_group, global_position_to_local_position_i(position), light_level, tmp) : false;
}

static struct block_group_future load_or_generate_block(ivec3_t position, struct voxy_block_database *block_database, struct voxy_block_generator *block_generator, const struct voxy_context *context, size_t *load_count, size_t *generate_count)
{
  profile_scope;

  struct block_group_future result;

  result = voxy_block_database_load(block_database, position);
  if(result.value || result.pending)
  {
    *load_count += !result.pending;
    return result;
  }

  result = voxy_block_generator_generate(block_generator, position, context);
  if(result.value || result.pending)
  {
    *generate_count += !result.pending;
    return result;
  }

  return block_group_future_ready(NULL);
}

static void load_or_generate_blocks(struct voxy_block_manager *block_manager, struct voxy_chunk_manager *chunk_manager, struct voxy_block_database *block_database , struct voxy_block_generator *block_generator, struct voxy_light_manager *light_manager, const struct voxy_context *context)
{
  profile_begin();

  size_t load_count = 0;
  size_t generate_count = 0;

  for(ptrdiff_t i=0; i<hmlen(chunk_manager->active_chunks); ++i)
  {
    ivec3_t position = chunk_manager->active_chunks[i].key;

    struct voxy_block_group *block_group;
    if((block_group = voxy_block_group_hash_table_lookup(&block_manager->block_groups, position)))
      continue;

    struct block_group_future block_group_future = load_or_generate_block(position, block_database, block_generator, context, &load_count, &generate_count);
    if((block_group = block_group_future.value))
    {
      voxy_block_group_hash_table_insert_unchecked(&block_manager->block_groups, block_group);

      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if((block_group->neighbours[direction] = voxy_block_group_hash_table_lookup(&block_manager->block_groups, ivec3_add(block_group->position, direction_as_ivec(direction)))))
          block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = block_group;

      for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
        for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
          for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
          {
            const ivec3_t local_position = ivec3(x, y, z);
            const ivec3_t global_position = local_position_to_global_position_i(local_position, block_group->position);
            if(voxy_block_group_get_block_light_level(block_group, local_position) != 0)
              voxy_light_manager_enqueue_creation_update(light_manager, block_manager, global_position);
            else if(z == 0 || z == VOXY_CHUNK_WIDTH - 1 || y == 0 || y == VOXY_CHUNK_WIDTH - 1 || x == 0 || x == VOXY_CHUNK_WIDTH - 1)
              voxy_light_manager_enqueue_destruction_update(light_manager, block_manager, global_position, 0);
          }
    }
  }

  if(generate_count != 0) LOG_INFO("Block Manager: Generated %zu block groups", generate_count);
  if(load_count != 0) LOG_INFO("Block Manager: Loaded %zu block groups from disk", load_count);

  profile_end("load_count", tformat("%zu", load_count),
              "generate_count", tformat("%zu", generate_count),
              "total_count", tformat("%zu", load_count + generate_count));
}

static void flush_blocks(struct voxy_block_manager *block_manager, struct voxy_block_database *block_database, libnet_server_t server)
{
  profile_begin();

  size_t save_count = 0;
  size_t send_count = 0;

  struct voxy_block_group *block_group;
  SC_HASH_TABLE_FOREACH(block_manager->block_groups, block_group)
  {
    if(block_group->disk_dirty)
    {
      struct unit_future result = voxy_block_database_save(block_database, block_group);
      if(!result.pending)
      {
        block_group->disk_dirty = false;
        save_count += 1;
      }
    }

    if(block_group->network_dirty)
    {
      voxy_block_network_update(block_group, server);
      block_group->network_dirty = false;
      send_count += 1;
    }
  }

  if(save_count != 0) LOG_INFO("Block Manager: Saved %zu block groups to disk", save_count);
  if(send_count != 0) LOG_INFO("Block Manager: Sent %zu block groups over the network", send_count);

  profile_end("send_count", tformat("%zu", send_count),
              "save_count", tformat("%zu", save_count));
}

static void discard_blocks(struct voxy_block_manager *block_manager, struct voxy_chunk_manager *chunk_manager, libnet_server_t server)
{
  profile_begin();

  size_t discard_count = 0;

  for(size_t i=0; i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(block_manager->block_groups.bucket_order); ++i)
  {
    struct voxy_block_group **block_group = &block_manager->block_groups.buckets[i].head;
    while(*block_group)
      if(hmgeti(chunk_manager->active_chunks, (*block_group)->position) == -1 && !(*block_group)->disk_dirty)
      {
        struct voxy_block_group *old_block_group = *block_group;
        *block_group = (*block_group)->next;

        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          if(old_block_group->neighbours[direction])
            old_block_group->neighbours[direction]->neighbours[direction_reverse(direction)] = NULL;

        voxy_block_network_remove(old_block_group, server);
        voxy_block_group_destroy(old_block_group);
      }
      else
        block_group = &(*block_group)->next;
  }

  if(discard_count != 0) LOG_INFO("Block Manager: Discarded %zu block groups", discard_count);

  profile_end("discard_count", tformat("%zu", discard_count));
}

void voxy_block_manager_update(struct voxy_block_manager *block_manager, struct voxy_chunk_manager *chunk_manager, struct voxy_block_database *block_database, struct voxy_block_generator *block_generator, struct voxy_light_manager *light_manager, libnet_server_t server, const struct voxy_context *context)
{
  profile_scope;

  load_or_generate_blocks(block_manager, chunk_manager, block_database, block_generator, light_manager, context);
  flush_blocks(block_manager, block_database, server);
  discard_blocks(block_manager, chunk_manager, server);
}

void voxy_block_manager_on_client_connected(struct voxy_block_manager *block_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_block_group *block_group;
  SC_HASH_TABLE_FOREACH(block_manager->block_groups, block_group)
  {
    struct voxy_server_block_group_update_message message;
    message.message.message.size = LIBNET_MESSAGE_SIZE(message);
    message.message.tag = VOXY_SERVER_MESSAGE_CHUNK_UPDATE;
    message.position = block_group->position;
    memcpy(&message.block_ids, &block_group->ids, sizeof block_group->ids);
    memcpy(&message.block_light_levels, &block_group->light_levels, sizeof block_group->light_levels);
    libnet_server_send_message(server, client_proxy, &message.message.message);
  }
}
