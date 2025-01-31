#include "manager.h"

#include "chunk/coordinates.h"

#include <libcore/profile.h>
#include <libcore/log.h>
#include <libcore/format.h>

#include <stdatomic.h>
#include <string.h>

void voxy_light_manager_init(struct voxy_light_manager *light_manager)
{
  DYNAMIC_ARRAY_INIT(light_manager->light_creation_updates);
  DYNAMIC_ARRAY_INIT(light_manager->light_destruction_updates);
}

void voxy_light_manager_fini(struct voxy_light_manager *light_manager)
{
  DYNAMIC_ARRAY_CLEAR(light_manager->light_destruction_updates);
  DYNAMIC_ARRAY_CLEAR(light_manager->light_creation_updates);
}

void voxy_light_manager_enqueue_destruction_update_at(struct voxy_light_manager *light_manager, struct voxy_chunk *chunk, ivec3_t position, uint8_t light_level)
{
  DYNAMIC_ARRAY_APPEND(light_manager->light_destruction_updates, ((struct light_destruction_update){
      .chunk = chunk,
      .x = position.x,
      .y = position.y,
      .z = position.z,
      .old_light_level = light_level,
  }));
}

void voxy_light_manager_enqueue_creation_update_at(struct voxy_light_manager *light_manager, struct voxy_chunk *chunk, ivec3_t position)
{
  DYNAMIC_ARRAY_APPEND(light_manager->light_destruction_updates, ((struct light_destruction_update){
      .chunk = chunk,
      .x = position.x,
      .y = position.y,
      .z = position.z,
  }));
}


void voxy_light_manager_enqueue_destruction_update(struct voxy_light_manager *light_manager, struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t light_level)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  struct voxy_chunk *chunk;
  if(!(chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, chunk_position)))
    return;

  voxy_light_manager_enqueue_destruction_update_at(light_manager, chunk, local_position, light_level);
}

void voxy_light_manager_enqueue_creation_update(struct voxy_light_manager *light_manager, struct voxy_chunk_manager *chunk_manager, ivec3_t position)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  struct voxy_chunk *chunk;
  if(!(chunk = voxy_chunk_hash_table_lookup(&chunk_manager->chunks, chunk_position)))
    return;

  voxy_light_manager_enqueue_creation_update_at(light_manager, chunk, local_position);
}

/// Compute new light level after propagation in direction.
static uint8_t propagate(uint8_t light_level, direction_t direction)
{
  if(direction == DIRECTION_BOTTOM && light_level == 15)
    return 15;

  if(light_level != 0)
    return light_level - 1;

  return 0;
}

static void process_light_destruction_update(
    struct voxy_block_registry *block_registry,
    struct light_destruction_updates *new_light_destruction_updates,
    struct light_creation_updates *new_light_creation_updates,
    struct light_destruction_update update, direction_t direction)
{
  struct voxy_chunk *neighbour_chunk = update.chunk;
  ivec3_t neighbour_position = ivec3(update.x, update.y, update.z);
  switch(direction)
  {
  case DIRECTION_LEFT:
    if(neighbour_position.x == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_LEFT];
      neighbour_position.x = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.x -= 1;
    break;
  case DIRECTION_RIGHT:
    if(neighbour_position.x == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_RIGHT];
      neighbour_position.x = 0;
    }
    else
      neighbour_position.x += 1;
    break;

  case DIRECTION_BACK:
    if(neighbour_position.y == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_BACK];
      neighbour_position.y = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.y -= 1;
    break;
  case DIRECTION_FRONT:
    if(neighbour_position.y == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_FRONT];
      neighbour_position.y = 0;
    }
    else
      neighbour_position.y += 1;
    break;

  case DIRECTION_BOTTOM:
    if(neighbour_position.z == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_BOTTOM];
      neighbour_position.z = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.z -= 1;
    break;
  case DIRECTION_TOP:
    if(neighbour_position.z == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_TOP];
      neighbour_position.z = 0;
    }
    else
      neighbour_position.z += 1;
    break;
  default:
    assert(0 && "Unreachable");
  }

  if(!neighbour_chunk)
    return;

  const uint8_t neighbour_id = voxy_chunk_get_block_id(neighbour_chunk, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t tmp;

  uint8_t neighbour_light_level;
  voxy_chunk_get_block_light_level_atomic(neighbour_chunk, neighbour_position, &neighbour_light_level, &tmp);

  while(neighbour_light_level != 0)
    if(neighbour_light_level == propagate(update.old_light_level, direction))
    {
      uint8_t neighbour_old_light_level = neighbour_light_level;
      neighbour_light_level = 0;
      if(!voxy_chunk_set_block_light_level_atomic(neighbour_chunk, neighbour_position, &neighbour_light_level, &tmp))
        continue;

      struct light_destruction_update new_update;
      new_update.chunk = neighbour_chunk;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      new_update.old_light_level = neighbour_old_light_level;
      DYNAMIC_ARRAY_APPEND(*new_light_destruction_updates, new_update);
      return;
    }
    else
    {
      struct light_creation_update new_update;
      new_update.chunk = neighbour_chunk;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      DYNAMIC_ARRAY_APPEND(*new_light_creation_updates, new_update);
      return;
    }
}

static void process_light_creation_update(
    struct voxy_block_registry *block_registry,
    struct light_creation_updates *new_light_creation_updates,
    struct light_creation_update update, direction_t direction)
{
  struct voxy_chunk *neighbour_chunk = update.chunk;
  ivec3_t neighbour_position = ivec3(update.x, update.y, update.z);
  switch(direction)
  {
  case DIRECTION_LEFT:
    if(neighbour_position.x == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_LEFT];
      neighbour_position.x = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.x -= 1;
    break;
  case DIRECTION_RIGHT:
    if(neighbour_position.x == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_RIGHT];
      neighbour_position.x = 0;
    }
    else
      neighbour_position.x += 1;
    break;

  case DIRECTION_BACK:
    if(neighbour_position.y == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_BACK];
      neighbour_position.y = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.y -= 1;
    break;
  case DIRECTION_FRONT:
    if(neighbour_position.y == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_FRONT];
      neighbour_position.y = 0;
    }
    else
      neighbour_position.y += 1;
    break;

  case DIRECTION_BOTTOM:
    if(neighbour_position.z == 0)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_BOTTOM];
      neighbour_position.z = VOXY_CHUNK_WIDTH - 1;
    }
    else
      neighbour_position.z -= 1;
    break;
  case DIRECTION_TOP:
    if(neighbour_position.z == VOXY_CHUNK_WIDTH - 1)
    {
      neighbour_chunk = neighbour_chunk->neighbours[DIRECTION_TOP];
      neighbour_position.z = 0;
    }
    else
      neighbour_position.z += 1;
    break;
  default:
    assert(0 && "Unreachable");
  }

  if(!neighbour_chunk)
    return;

  const uint8_t neighbour_id = voxy_chunk_get_block_id(neighbour_chunk, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t tmp;

  uint8_t light_level;
  voxy_chunk_get_block_light_level_atomic(update.chunk, ivec3(update.x, update.y, update.z), &light_level, &tmp);

  uint8_t neighbour_light_level;
  voxy_chunk_get_block_light_level_atomic(neighbour_chunk, neighbour_position, &neighbour_light_level, &tmp);

  while(neighbour_light_level < propagate(light_level, direction))
  {
    neighbour_light_level = propagate(light_level, direction);
    if(voxy_chunk_set_block_light_level_atomic(neighbour_chunk, neighbour_position, &neighbour_light_level, &tmp))
    {
      struct light_creation_update new_update;
      new_update.chunk = neighbour_chunk;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      DYNAMIC_ARRAY_APPEND(*new_light_creation_updates, new_update);
      return;
    }
  }
}

static void process_light_destruction_updates(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry)
{
  profile_begin();

  size_t count = 0;
  while(light_manager->light_destruction_updates.item_count != 0)
    #pragma omp parallel
    {
      struct light_destruction_updates new_light_destruction_updates = {0};
      struct light_creation_updates new_light_creation_updates = {0};

      #pragma omp for
      for(size_t i=0; i<light_manager->light_destruction_updates.item_count; ++i)
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_destruction_update(block_registry, &new_light_destruction_updates, &new_light_creation_updates, light_manager->light_destruction_updates.items[i], direction);

      #pragma omp single
      {
        count += light_manager->light_destruction_updates.item_count;
        light_manager->light_destruction_updates.item_count = 0;
      }

      const size_t light_creation_index    = atomic_fetch_add_explicit((_Atomic size_t *)&light_manager->light_creation_updates.item_count,    new_light_creation_updates.item_count,    memory_order_relaxed);
      const size_t light_destruction_index = atomic_fetch_add_explicit((_Atomic size_t *)&light_manager->light_destruction_updates.item_count, new_light_destruction_updates.item_count, memory_order_relaxed);

      #pragma omp barrier
      #pragma omp single
      {
        DYNAMIC_ARRAY_RESERVE(light_manager->light_creation_updates,    light_manager->light_creation_updates.item_count);
        DYNAMIC_ARRAY_RESERVE(light_manager->light_destruction_updates, light_manager->light_destruction_updates.item_count);
      }

      memcpy(&light_manager->light_creation_updates.items[light_creation_index], new_light_creation_updates.items, new_light_creation_updates.item_count * sizeof new_light_creation_updates.items[0]);
      free(new_light_creation_updates.items);

      memcpy(&light_manager->light_destruction_updates.items[light_destruction_index], new_light_destruction_updates.items, new_light_destruction_updates.item_count * sizeof new_light_destruction_updates.items[0]);
      free(new_light_destruction_updates.items);
    }

  if(count != 0)
    LOG_INFO("Processed %zu light destruction updates", count);

  profile_end("count", tformat("%zd", count));
}

static void process_light_creation_updates(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry)
{
  profile_begin();

  size_t count = 0;
  while(light_manager->light_creation_updates.item_count != 0)
    #pragma omp parallel
    {
      struct light_creation_updates new_light_creation_updates = {0};

      #pragma omp for
      for(size_t i=0; i<light_manager->light_creation_updates.item_count; ++i)
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_creation_update(block_registry, &new_light_creation_updates, light_manager->light_creation_updates.items[i], direction);

      #pragma omp single
      {
        count += light_manager->light_creation_updates.item_count;
        light_manager->light_creation_updates.item_count = 0;
      }

      const size_t light_creation_index = atomic_fetch_add_explicit((_Atomic size_t *)&light_manager->light_creation_updates.item_count, new_light_creation_updates.item_count,    memory_order_relaxed);

      #pragma omp barrier
      #pragma omp single
      {
        DYNAMIC_ARRAY_RESERVE(light_manager->light_creation_updates, light_manager->light_creation_updates.item_count);
      }

      memcpy(&light_manager->light_creation_updates.items[light_creation_index], new_light_creation_updates.items, new_light_creation_updates.item_count * sizeof new_light_creation_updates.items[0]);
      free(new_light_creation_updates.items);
    }

  if(count != 0)
    LOG_INFO("Processed %zu light creation updates", count);

  profile_end("count", tformat("%zd", count));
}

void voxy_light_manager_update(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry)
{
  profile_scope;

  process_light_destruction_updates(light_manager, block_registry);
  process_light_creation_updates(light_manager, block_registry);
}
