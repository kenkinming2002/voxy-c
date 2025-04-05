#include "manager.h"

#include "chunk/coordinates.h"

#include <libcore/profile.h>
#include <libcore/log.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <stdatomic.h>
#include <string.h>

void voxy_light_manager_init(struct voxy_light_manager *light_manager)
{
  light_manager->light_creation_updates = NULL;
  light_manager->light_destruction_updates = NULL;
}

void voxy_light_manager_fini(struct voxy_light_manager *light_manager)
{
  arrfree(light_manager->light_creation_updates);
  arrfree(light_manager->light_destruction_updates);
}

void voxy_light_manager_enqueue_destruction_update_at(struct voxy_light_manager *light_manager, struct voxy_block_group *block_group, ivec3_t position, uint8_t light_level)
{
  arrput(light_manager->light_destruction_updates, ((struct light_destruction_update){
      .block_group = block_group,
      .x = position.x,
      .y = position.y,
      .z = position.z,
      .old_light_level = light_level,
  }));
}

void voxy_light_manager_enqueue_creation_update_at(struct voxy_light_manager *light_manager, struct voxy_block_group *block_group, ivec3_t position)
{
  arrput(light_manager->light_creation_updates, ((struct light_creation_update){
      .block_group = block_group,
      .x = position.x,
      .y = position.y,
      .z = position.z,
  }));
}


void voxy_light_manager_enqueue_destruction_update(struct voxy_light_manager *light_manager, struct voxy_block_manager *block_manager, ivec3_t position, uint8_t light_level)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  ptrdiff_t i = hmgeti(block_manager->block_group_nodes, chunk_position);
  if(i == -1)
    return;

  voxy_light_manager_enqueue_destruction_update_at(light_manager, block_manager->block_group_nodes[i].value, local_position, light_level);
}

void voxy_light_manager_enqueue_creation_update(struct voxy_light_manager *light_manager, struct voxy_block_manager *block_manager, ivec3_t position)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  ptrdiff_t i = hmgeti(block_manager->block_group_nodes, chunk_position);
  if(i == -1)
    return;

  voxy_light_manager_enqueue_creation_update_at(light_manager, block_manager->block_group_nodes[i].value, local_position);
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

struct cursor
{
  struct voxy_block_group *block_group;
  ivec3_t position;
};

static inline struct cursor traverse(struct cursor cursor, direction_t direction)
{
  switch(direction)
  {
  case DIRECTION_LEFT:
    if(cursor.position.x == 0)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_LEFT];
      cursor.position.x = VOXY_CHUNK_WIDTH - 1;
    }
    else
      cursor.position.x -= 1;
    break;
  case DIRECTION_RIGHT:
    if(cursor.position.x == VOXY_CHUNK_WIDTH - 1)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_RIGHT];
      cursor.position.x = 0;
    }
    else
      cursor.position.x += 1;
    break;

  case DIRECTION_BACK:
    if(cursor.position.y == 0)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_BACK];
      cursor.position.y = VOXY_CHUNK_WIDTH - 1;
    }
    else
      cursor.position.y -= 1;
    break;
  case DIRECTION_FRONT:
    if(cursor.position.y == VOXY_CHUNK_WIDTH - 1)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_FRONT];
      cursor.position.y = 0;
    }
    else
      cursor.position.y += 1;
    break;

  case DIRECTION_BOTTOM:
    if(cursor.position.z == 0)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_BOTTOM];
      cursor.position.z = VOXY_CHUNK_WIDTH - 1;
    }
    else
      cursor.position.z -= 1;
    break;
  case DIRECTION_TOP:
    if(cursor.position.z == VOXY_CHUNK_WIDTH - 1)
    {
      cursor.block_group = cursor.block_group->neighbours[DIRECTION_TOP];
      cursor.position.z = 0;
    }
    else
      cursor.position.z += 1;
    break;
  default:
    assert(0 && "Unreachable");
  }
  return cursor;
}

static inline void process_light_destruction_update(
    struct voxy_block_registry *block_registry,
    struct light_destruction_update **new_light_destruction_updates,
    struct light_creation_update **new_light_creation_updates,
    struct light_destruction_update update, direction_t direction)
{
  struct cursor neighbour_cursor = traverse((struct cursor) { update.block_group, ivec3(update.x, update.y, update.z) } , direction);
  struct voxy_block_group *neighbour_block_group = neighbour_cursor.block_group;
  ivec3_t neighbour_position = neighbour_cursor.position;
  if(!neighbour_block_group)
    return;

  const uint8_t neighbour_id = voxy_block_group_get_block_id(neighbour_block_group, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t tmp;

  uint8_t neighbour_light_level;
  voxy_block_group_get_block_light_level_atomic(neighbour_block_group, neighbour_position, &neighbour_light_level, &tmp);

  while(neighbour_light_level != 0)
    if(neighbour_light_level == propagate(update.old_light_level, direction))
    {
      uint8_t neighbour_old_light_level = neighbour_light_level;
      neighbour_light_level = 0;
      if(!voxy_block_group_set_block_light_level_atomic(neighbour_block_group, neighbour_position, &neighbour_light_level, &tmp))
        continue;

      struct light_destruction_update new_update;
      new_update.block_group = neighbour_block_group;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      new_update.old_light_level = neighbour_old_light_level;
      arrput(*new_light_destruction_updates, new_update);
      return;
    }
    else
    {
      struct light_creation_update new_update;
      new_update.block_group = neighbour_block_group;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      arrput(*new_light_creation_updates, new_update);
      return;
    }
}

static inline void process_light_creation_update(
    struct voxy_block_registry *block_registry,
    struct light_creation_update **new_light_creation_updates,
    struct light_creation_update update, direction_t direction)
{
  struct cursor neighbour_cursor = traverse((struct cursor) { update.block_group, ivec3(update.x, update.y, update.z) } , direction);
  struct voxy_block_group *neighbour_block_group = neighbour_cursor.block_group;
  ivec3_t neighbour_position = neighbour_cursor.position;
  if(!neighbour_block_group)
    return;

  const uint8_t neighbour_id = voxy_block_group_get_block_id(neighbour_block_group, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_block_registry_query_block(block_registry, neighbour_id);
  if(neighbour_info.collide)
    return;

  uint8_t tmp;

  uint8_t light_level;
  voxy_block_group_get_block_light_level_atomic(update.block_group, ivec3(update.x, update.y, update.z), &light_level, &tmp);

  uint8_t neighbour_light_level;
  voxy_block_group_get_block_light_level_atomic(neighbour_block_group, neighbour_position, &neighbour_light_level, &tmp);

  while(neighbour_light_level < propagate(light_level, direction))
  {
    neighbour_light_level = propagate(light_level, direction);
    if(voxy_block_group_set_block_light_level_atomic(neighbour_block_group, neighbour_position, &neighbour_light_level, &tmp))
    {
      struct light_creation_update new_update;
      new_update.block_group = neighbour_block_group;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      arrput(*new_light_creation_updates, new_update);
      return;
    }
  }
}

static void process_light_destruction_updates(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry)
{
  profile_begin();

  size_t count = 0;
  while(arrlenu(light_manager->light_destruction_updates) != 0)
  {
    const size_t light_destruction_update_count = arrlenu(light_manager->light_destruction_updates);
    arrsetlen(light_manager->light_destruction_updates, 0);
    count += light_destruction_update_count;

    _Atomic size_t new_light_creation_update_count = arrlenu(light_manager->light_creation_updates);
    _Atomic size_t new_light_destruction_update_count = 0;

    #pragma omp parallel
    {
      struct light_destruction_update *new_light_destruction_updates = NULL;
      struct light_creation_update *new_light_creation_updates = NULL;

      #pragma omp for
      for(size_t i=0; i<light_destruction_update_count; ++i)
        #pragma omp unroll
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_destruction_update(block_registry, &new_light_destruction_updates, &new_light_creation_updates, light_manager->light_destruction_updates[i], direction);

      const size_t light_creation_index    = atomic_fetch_add_explicit(&new_light_creation_update_count,    arrlenu(new_light_creation_updates),    memory_order_relaxed);
      const size_t light_destruction_index = atomic_fetch_add_explicit(&new_light_destruction_update_count, arrlenu(new_light_destruction_updates), memory_order_relaxed);
      #pragma omp barrier
      #pragma omp single
      {
        arrsetlen(light_manager->light_creation_updates,    new_light_creation_update_count);
        arrsetlen(light_manager->light_destruction_updates, new_light_destruction_update_count);
      }

      memcpy(&light_manager->light_creation_updates   [light_creation_index],    new_light_creation_updates,    arrlenu(new_light_creation_updates)    * sizeof *new_light_creation_updates);
      memcpy(&light_manager->light_destruction_updates[light_destruction_index], new_light_destruction_updates, arrlenu(new_light_destruction_updates) * sizeof *new_light_destruction_updates);

      arrfree(new_light_creation_updates);
      arrfree(new_light_destruction_updates);
    }
  }

  if(count != 0)
    LOG_INFO("Processed %zu light destruction updates", count);

  profile_end("count", tformat("%zd", count));
}

static void process_light_creation_updates(struct voxy_light_manager *light_manager, struct voxy_block_registry *block_registry)
{
  profile_begin();

  size_t count = 0;
  while(arrlenu(light_manager->light_creation_updates) != 0)
  {
    const size_t light_creation_update_count = arrlenu(light_manager->light_creation_updates);
    arrsetlen(light_manager->light_creation_updates, 0);
    count += light_creation_update_count;

    _Atomic size_t new_light_creation_update_count = 0;

    #pragma omp parallel
    {
      struct light_creation_update *new_light_creation_updates = NULL;

      #pragma omp for
      for(size_t i=0; i<light_creation_update_count; ++i)
        #pragma omp unroll
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_creation_update(block_registry, &new_light_creation_updates, light_manager->light_creation_updates[i], direction);

      const size_t light_creation_index = atomic_fetch_add_explicit(&new_light_creation_update_count, arrlenu(new_light_creation_updates), memory_order_relaxed);
      #pragma omp barrier
      #pragma omp single
      {
        arrsetlen(light_manager->light_creation_updates, new_light_creation_update_count);
      }

      memcpy(&light_manager->light_creation_updates[light_creation_index], new_light_creation_updates, arrlenu(new_light_creation_updates) * sizeof *new_light_creation_updates);
      arrfree(new_light_creation_updates);
    }
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
