#include "manager.h"

#include "coordinates.h"
#include "database.h"
#include "network.h"

#include "registry/block.h"
#include "light/manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/format.h>

#include <string.h>
#include <stdlib.h>

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager)
{
  ivec3_hash_table_init(&chunk_manager->active_chunks);
  voxy_chunk_hash_table_init(&chunk_manager->chunks);
}

void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager)
{
  voxy_chunk_hash_table_dispose(&chunk_manager->chunks);
  ivec3_hash_table_dispose(&chunk_manager->active_chunks);
}

uint8_t voxy_chunk_manager_get_block_id(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? voxy_chunk_get_block_id(chunk, global_position_to_local_position_i(position)) : def;
}

uint8_t voxy_chunk_manager_get_block_light_level(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t def)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? voxy_chunk_get_block_light_level(chunk, global_position_to_local_position_i(position)) : def;
}

void voxy_chunk_manager_set_block_id(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t id)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
    voxy_chunk_set_block_id(chunk, global_position_to_local_position_i(position), id);
}

void voxy_chunk_manager_set_block_light_level(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t light_level)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
    voxy_chunk_set_block_light_level(chunk, global_position_to_local_position_i(position), light_level);
}

/// Set block at given position.
///
/// The light level of the block will be derived from light info in block
/// registry. This will also enqueue any necessary light updates to light
/// manager..
VOXY_SERVER_EXPORT void voxy_chunk_manager_set_block(
    struct voxy_chunk_manager *chunk_manager,
    struct voxy_block_registry *block_registry,
    struct voxy_light_manager *light_manager,
    ivec3_t position,
    uint8_t id)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(!chunk)
    return;

  const uint8_t old_light_level = voxy_chunk_get_block_light_level(chunk, global_position_to_local_position_i(position));
  const uint8_t light_level = voxy_block_registry_query_block(block_registry, id).light_level;

  voxy_chunk_set_block_id(chunk, global_position_to_local_position_i(position), id);
  voxy_chunk_set_block_light_level(chunk, global_position_to_local_position_i(position), light_level);

  if(old_light_level < light_level)
    voxy_light_manager_enqueue_creation_update(light_manager, chunk_manager, position);

  //// FIXME: Light destroy update really only support the case if light level
  ////        changes to 0. This happens to be the only possible cases for now.
  if(old_light_level >= light_level)
    voxy_light_manager_enqueue_destruction_update(light_manager, chunk_manager, position, old_light_level);
}

bool voxy_chunk_manager_get_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  if(chunk)
  {
    voxy_chunk_get_block_light_level_atomic(chunk, global_position_to_local_position_i(position), light_level, tmp);
    return true;
  }
  else
    return false;
}

bool voxy_chunk_manager_set_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  struct voxy_chunk *chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, get_chunk_position_i(position));
  return chunk ? voxy_chunk_set_block_light_level_atomic(chunk, global_position_to_local_position_i(position), light_level, tmp) : false;
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
    node->key = position;
    ivec3_hash_table_insert_unchecked(&chunk_manager->active_chunks, node);
  }
}

static struct chunk_future load_or_generate_chunk(ivec3_t position, struct voxy_chunk_database *chunk_database, struct voxy_chunk_generator *chunk_generator, const struct voxy_context *context, size_t *load_count, size_t *generate_count)
{
  profile_scope;

  struct chunk_future result;

  result = voxy_chunk_database_load(chunk_database, position);
  if(result.value || result.pending)
  {
    *load_count += !result.pending;
    return result;
  }

  result = voxy_chunk_generator_generate(chunk_generator, position, context);
  if(result.value || result.pending)
  {
    *generate_count += !result.pending;
    return result;
  }

  return chunk_future_ready(NULL);
}

static void load_or_generate_chunks(struct voxy_chunk_manager *chunk_manager, struct voxy_chunk_database *chunk_database , struct voxy_chunk_generator *chunk_generator, struct voxy_light_manager *light_manager, const struct voxy_context *context)
{
  profile_begin();

  size_t load_count = 0;
  size_t generate_count = 0;

  struct ivec3_node *position;
  SC_HASH_TABLE_FOREACH(chunk_manager->active_chunks, position)
  {
    struct voxy_chunk *chunk;
    if((chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, position->key)))
      continue;

    struct chunk_future chunk_future = load_or_generate_chunk(position->key, chunk_database, chunk_generator, context, &load_count, &generate_count);
    if((chunk = chunk_future.value))
    {
      voxy_chunk_hash_table_insert_unchecked(&chunk_manager->chunks, chunk);

      for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        if((chunk->neighbours[direction] = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, ivec3_add(chunk->position, direction_as_ivec(direction)))))
          chunk->neighbours[direction]->neighbours[direction_reverse(direction)] = chunk;

      for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
        for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
          for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
          {
            const ivec3_t local_position = ivec3(x, y, z);
            const ivec3_t global_position = local_position_to_global_position_i(local_position, chunk->position);
            if(voxy_chunk_get_block_light_level(chunk, local_position) != 0)
              voxy_light_manager_enqueue_creation_update(light_manager, chunk_manager, global_position);
            else if(z == 0 || z == VOXY_CHUNK_WIDTH - 1 || y == 0 || y == VOXY_CHUNK_WIDTH - 1 || x == 0 || x == VOXY_CHUNK_WIDTH - 1)
              voxy_light_manager_enqueue_destruction_update(light_manager, chunk_manager, global_position, 0);
          }
    }
  }

  if(generate_count != 0) LOG_INFO("Chunk Manager: Generated %zu chunks", generate_count);
  if(load_count != 0) LOG_INFO("Chunk Manager: Loaded %zu chunks from disk", load_count);

  profile_end("load_count", tformat("%zu", load_count),
              "generate_count", tformat("%zu", generate_count),
              "total_count", tformat("%zu", load_count + generate_count));
}

static void flush_chunks(struct voxy_chunk_manager *chunk_manager, struct voxy_chunk_database *chunk_database, libnet_server_t server)
{
  profile_begin();

  size_t save_count = 0;
  size_t send_count = 0;

  struct voxy_chunk *chunk;
  SC_HASH_TABLE_FOREACH(chunk_manager->chunks, chunk)
  {
    if(chunk->disk_dirty)
    {
      struct unit_future result = voxy_chunk_database_save(chunk_database, chunk);
      if(!result.pending)
      {
        chunk->disk_dirty = false;
        save_count += 1;
      }
    }

    if(chunk->network_dirty)
    {
      voxy_chunk_network_update(chunk, server);
      chunk->network_dirty = false;
      send_count += 1;
    }
  }

  if(save_count != 0) LOG_INFO("Chunk Manager: Saved %zu chunks to disk", save_count);
  if(send_count != 0) LOG_INFO("Chunk Manager: Sent %zu chunks over the network", send_count);

  profile_end("send_count", tformat("%zu", send_count),
              "save_count", tformat("%zu", save_count));
}

static void discard_chunks(struct voxy_chunk_manager *chunk_manager, libnet_server_t server)
{
  profile_begin();

  size_t discard_count = 0;

  for(size_t i=0; i<SC_HASH_TABLE_BUCKET_COUNT_FROM_ORDER(chunk_manager->chunks.bucket_order); ++i)
  {
    struct voxy_chunk **chunk = &chunk_manager->chunks.buckets[i].head;
    while(*chunk)
      if(!ivec3_hash_table_lookup(&chunk_manager->active_chunks, (*chunk)->position) && !(*chunk)->disk_dirty)
      {
        struct voxy_chunk *old_chunk = *chunk;
        *chunk = (*chunk)->next;

        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          if(old_chunk->neighbours[direction])
            old_chunk->neighbours[direction]->neighbours[direction_reverse(direction)] = NULL;

        voxy_chunk_network_remove(old_chunk, server);
        voxy_chunk_destroy(old_chunk);
      }
      else
        chunk = &(*chunk)->next;
  }

  if(discard_count != 0) LOG_INFO("Chunk Manager: Discarded %zu", discard_count);

  profile_end("discard_count", tformat("%zu", discard_count));
}

void voxy_chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct voxy_chunk_database *chunk_database, struct voxy_chunk_generator *chunk_generator, struct voxy_light_manager *light_manager, libnet_server_t server, const struct voxy_context *context)
{
  profile_scope;

  load_or_generate_chunks(chunk_manager, chunk_database, chunk_generator, light_manager, context);
  flush_chunks(chunk_manager, chunk_database, server);
  discard_chunks(chunk_manager, server);
}

void voxy_chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_chunk *chunk;
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
