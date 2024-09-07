#include "manager.h"

#include <libcommon/core/log.h>

#include <stdatomic.h>
#include <string.h>

void light_manager_init(struct light_manager *light_manager)
{
  DYNAMIC_ARRAY_INIT(light_manager->light_creation_updates);
  DYNAMIC_ARRAY_INIT(light_manager->light_destruction_updates);
}

void light_manager_fini(struct light_manager *light_manager)
{
  DYNAMIC_ARRAY_CLEAR(light_manager->light_destruction_updates);
  DYNAMIC_ARRAY_CLEAR(light_manager->light_creation_updates);
}

void light_manager_enqueue_destruction_update(struct light_manager *light_manager, ivec3_t position, uint8_t light_level)
{
  DYNAMIC_ARRAY_APPEND(light_manager->light_destruction_updates, ((struct light_destruction_update){ .position = position, .old_light_level = light_level, }));
}

void light_manager_enqueue_creation_update(struct light_manager *light_manager, ivec3_t position)
{
  DYNAMIC_ARRAY_APPEND(light_manager->light_creation_updates, ((struct light_creation_update){ .position = position,  }));
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
    struct voxy_chunk_manager *chunk_manager,
    struct light_destruction_updates *new_light_destruction_updates,
    struct light_creation_updates *new_light_creation_updates,
    struct light_destruction_update update, direction_t direction)
{
  const ivec3_t neighbour_position = ivec3_add(update.position, direction_as_ivec(direction));
  const uint8_t neighbour_id = chunk_manager_get_block_id(chunk_manager, neighbour_position, UINT8_MAX);
  if(neighbour_id == UINT8_MAX)
    return;

  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t neighbour_light_level;
  uint8_t tmp;
  if(!chunk_manager_get_block_light_level_atomic(chunk_manager, neighbour_position, &neighbour_light_level, &tmp))
    return;

  while(neighbour_light_level != 0)
    if(neighbour_light_level == propagate(update.old_light_level, direction))
    {
      uint8_t neighbour_old_light_level = neighbour_light_level;
      if(!chunk_manager_set_block_light_level_atomic(chunk_manager, neighbour_position, &neighbour_light_level, &tmp))
        continue;

      struct light_destruction_update new_update;
      new_update.position = neighbour_position;
      new_update.old_light_level = neighbour_old_light_level;
      DYNAMIC_ARRAY_APPEND(*new_light_destruction_updates, new_update);
      return;
    }
    else
    {
      struct light_creation_update new_update;
      new_update.position = neighbour_position;
      DYNAMIC_ARRAY_APPEND(*new_light_creation_updates, new_update);
      return;
    }
}

static void process_light_creation_update(
    struct voxy_block_registry *block_registry,
    struct voxy_chunk_manager *chunk_manager,
    struct light_creation_updates *new_light_creation_updates,
    struct light_creation_update update, direction_t direction)
{
  const ivec3_t neighbour_position = ivec3_add(update.position, direction_as_ivec(direction));
  const uint8_t neighbour_id = chunk_manager_get_block_id(chunk_manager, neighbour_position, UINT8_MAX);
  if(neighbour_id == UINT8_MAX)
    return;

  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t tmp;

  uint8_t light_level;
  if(!chunk_manager_get_block_light_level_atomic(chunk_manager, update.position, &light_level, &tmp))
    return;

  uint8_t neighbour_light_level;
  if(!chunk_manager_get_block_light_level_atomic(chunk_manager, neighbour_position, &neighbour_light_level, &tmp))
    return;

  while(neighbour_light_level < propagate(light_level, direction))
  {
    neighbour_light_level = propagate(light_level, direction);
    if(chunk_manager_set_block_light_level_atomic(chunk_manager, neighbour_position, &neighbour_light_level, &tmp))
    {
      struct light_creation_update new_update;
      new_update.position = neighbour_position;
      DYNAMIC_ARRAY_APPEND(*new_light_creation_updates, new_update);
      return;
    }
  }
}

static void process_light_destruction_updates(struct light_manager *light_manager, struct voxy_block_registry *block_registry, struct voxy_chunk_manager *chunk_manager)
{
  size_t count = 0;
  while(light_manager->light_destruction_updates.item_count != 0)
    #pragma omp parallel
    {
      struct light_destruction_updates new_light_destruction_updates = {0};
      struct light_creation_updates new_light_creation_updates = {0};

      #pragma omp for
      for(size_t i=0; i<light_manager->light_destruction_updates.item_count; ++i)
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_destruction_update(block_registry, chunk_manager, &new_light_destruction_updates, &new_light_creation_updates, light_manager->light_destruction_updates.items[i], direction);

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
}

static void process_light_creation_updates(struct light_manager *light_manager, struct voxy_block_registry *block_registry, struct voxy_chunk_manager *chunk_manager)
{
  size_t count = 0;
  while(light_manager->light_creation_updates.item_count != 0)
    #pragma omp parallel
    {
      struct light_creation_updates new_light_creation_updates = {0};

      #pragma omp for
      for(size_t i=0; i<light_manager->light_creation_updates.item_count; ++i)
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_creation_update(block_registry, chunk_manager, &new_light_creation_updates, light_manager->light_creation_updates.items[i], direction);

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
}

void light_manager_update(struct light_manager *light_manager, struct voxy_block_registry *block_registry, struct voxy_chunk_manager *chunk_manager)
{
  process_light_destruction_updates(light_manager, block_registry, chunk_manager);
  process_light_creation_updates(light_manager, block_registry, chunk_manager);
}
