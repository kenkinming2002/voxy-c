#include "light.h"

#include <voxy/config.h>
#include <voxy/server/chunk/block/manager.h>
#include <voxy/server/registry/block.h>

#include "chunk/block/manager.h"
#include "chunk/coordinates.h"
#include "chunk/block/group.h"

#include <libcore/profile.h>
#include <libcore/log.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <stdatomic.h>
#include <string.h>

/// References:
///  - https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
///  - https://www.seedofandromeda.com/blogs/30-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-2
/// The link is dead but the article could be still be accessed via wayback machine.

struct light_destruction_update
{
  struct voxy_block_group *block_group;
  uint8_t x;
  uint8_t y;
  uint8_t z;
  voxy_light_t old_light;
};

struct light_creation_update
{
  struct voxy_block_group *block_group;
  uint8_t x;
  uint8_t y;
  uint8_t z;
};

static struct light_creation_update *light_creation_updates;
static struct light_destruction_update *light_destruction_updates;

void enqueue_light_destruction_update_at(struct voxy_block_group *block_group, ivec3_t position, voxy_light_t light)
{
  arrput(light_destruction_updates, ((struct light_destruction_update){
      .block_group = block_group,
      .x = position.x,
      .y = position.y,
      .z = position.z,
      .old_light = light,
  }));
}

void enqueue_light_creation_update_at(struct voxy_block_group *block_group, ivec3_t position)
{
  arrput(light_creation_updates, ((struct light_creation_update){
      .block_group = block_group,
      .x = position.x,
      .y = position.y,
      .z = position.z,
  }));
}


void enqueue_light_destruction_update(ivec3_t position, voxy_light_t light)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  struct voxy_block_group *block_group = voxy_get_block_group(chunk_position);
  if(block_group)
    enqueue_light_destruction_update_at(block_group, local_position, light);
}

void enqueue_light_creation_update(ivec3_t position)
{
  ivec3_t chunk_position = get_chunk_position_i(position);
  ivec3_t local_position = global_position_to_local_position_i(position);

  struct voxy_block_group *block_group = voxy_get_block_group(chunk_position);
  if(block_group)
    enqueue_light_creation_update_at(block_group, local_position);
}

static bool light_eql(voxy_light_t lhs, voxy_light_t rhs)
{
  return lhs.level == rhs.level && (bool)lhs.sol == (bool)rhs.sol;
}

/// Compute new light level after propagation in direction.
static voxy_light_t propagate(voxy_light_t light, direction_t direction)
{
  if(direction == DIRECTION_BOTTOM && light.sol)
    return light;

  light.sol = 0;
  if(light.level != 0)
    --light.level;

  return light;
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
    struct light_destruction_update **new_light_destruction_updates,
    struct light_creation_update **new_light_creation_updates,
    struct light_destruction_update update, direction_t direction)
{
  struct cursor neighbour_cursor = traverse((struct cursor) { update.block_group, ivec3(update.x, update.y, update.z) } , direction);
  struct voxy_block_group *neighbour_block_group = neighbour_cursor.block_group;
  ivec3_t neighbour_position = neighbour_cursor.position;
  if(!neighbour_block_group)
    return;

  const uint8_t neighbour_id = voxy_block_group_get_id(neighbour_block_group, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_query_block(neighbour_id);
  if(neighbour_info.collide)
    return;

  voxy_light_t neighbour_light = voxy_block_group_get_light_atomic(neighbour_block_group, neighbour_position);
  if(neighbour_light.level != 0)
  {
    if(light_eql(neighbour_light, propagate(update.old_light, direction)))
    {
      voxy_block_group_set_light_atomic(neighbour_block_group, neighbour_position, (voxy_light_t){ .level = 0, .sol = 0 });

      struct light_destruction_update new_update;
      new_update.block_group = neighbour_block_group;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      new_update.old_light = neighbour_light;
      arrput(*new_light_destruction_updates, new_update);
    }
    else
    {
      struct light_creation_update new_update;
      new_update.block_group = neighbour_block_group;
      new_update.x = neighbour_position.x;
      new_update.y = neighbour_position.y;
      new_update.z = neighbour_position.z;
      arrput(*new_light_creation_updates, new_update);
    }
  }
}

static inline void process_light_creation_update(
    struct light_creation_update **new_light_creation_updates,
    struct light_creation_update update, direction_t direction)
{
  struct cursor neighbour_cursor = traverse((struct cursor) { update.block_group, ivec3(update.x, update.y, update.z) } , direction);
  struct voxy_block_group *neighbour_block_group = neighbour_cursor.block_group;
  ivec3_t neighbour_position = neighbour_cursor.position;
  if(!neighbour_block_group)
    return;

  const voxy_block_id_t neighbour_id = voxy_block_group_get_id(neighbour_block_group, neighbour_position);
  const struct voxy_block_info neighbour_info = voxy_query_block(neighbour_id);
  if(neighbour_info.collide)
    return;

  voxy_light_t light = voxy_block_group_get_light_atomic(update.block_group, ivec3(update.x, update.y, update.z));
  voxy_light_t neighbour_light = voxy_block_group_get_light_atomic(neighbour_block_group, neighbour_position);
  voxy_light_t propagated_light = propagate(light, direction);
  if(neighbour_light.level < propagated_light.level)
  {
    voxy_block_group_set_light_atomic(neighbour_block_group, neighbour_position, propagated_light);

    struct light_creation_update new_update;
    new_update.block_group = neighbour_block_group;
    new_update.x = neighbour_position.x;
    new_update.y = neighbour_position.y;
    new_update.z = neighbour_position.z;
    arrput(*new_light_creation_updates, new_update);
  }
}

static void process_light_destruction_updates(void)
{
  profile_begin();

  size_t count = 0;
  while(arrlenu(light_destruction_updates) != 0)
  {
    const size_t light_destruction_update_count = arrlenu(light_destruction_updates);
    arrsetlen(light_destruction_updates, 0);
    count += light_destruction_update_count;

    _Atomic size_t new_light_creation_update_count = arrlenu(light_creation_updates);
    _Atomic size_t new_light_destruction_update_count = 0;

    #pragma omp parallel
    {
      struct light_destruction_update *new_light_destruction_updates = NULL;
      struct light_creation_update *new_light_creation_updates = NULL;

      #pragma omp for
      for(size_t i=0; i<light_destruction_update_count; ++i)
        #pragma omp unroll
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_destruction_update(&new_light_destruction_updates, &new_light_creation_updates, light_destruction_updates[i], direction);

      const size_t light_creation_index    = atomic_fetch_add_explicit(&new_light_creation_update_count,    arrlenu(new_light_creation_updates),    memory_order_relaxed);
      const size_t light_destruction_index = atomic_fetch_add_explicit(&new_light_destruction_update_count, arrlenu(new_light_destruction_updates), memory_order_relaxed);
      #pragma omp barrier
      #pragma omp single
      {
        arrsetlen(light_creation_updates,    new_light_creation_update_count);
        arrsetlen(light_destruction_updates, new_light_destruction_update_count);
      }

      memcpy(&light_creation_updates   [light_creation_index],    new_light_creation_updates,    arrlenu(new_light_creation_updates)    * sizeof *new_light_creation_updates);
      memcpy(&light_destruction_updates[light_destruction_index], new_light_destruction_updates, arrlenu(new_light_destruction_updates) * sizeof *new_light_destruction_updates);

      arrfree(new_light_creation_updates);
      arrfree(new_light_destruction_updates);
    }
  }

  if(count != 0)
    LOG_INFO("Processed %zu light destruction updates", count);

  profile_end("count", tformat("%zd", count));
}

static void process_light_creation_updates(void)
{
  profile_begin();

  size_t count = 0;
  while(arrlenu(light_creation_updates) != 0)
  {
    const size_t light_creation_update_count = arrlenu(light_creation_updates);
    arrsetlen(light_creation_updates, 0);
    count += light_creation_update_count;

    _Atomic size_t new_light_creation_update_count = 0;

    #pragma omp parallel
    {
      struct light_creation_update *new_light_creation_updates = NULL;

      #pragma omp for
      for(size_t i=0; i<light_creation_update_count; ++i)
        #pragma omp unroll
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
          process_light_creation_update(&new_light_creation_updates, light_creation_updates[i], direction);

      const size_t light_creation_index = atomic_fetch_add_explicit(&new_light_creation_update_count, arrlenu(new_light_creation_updates), memory_order_relaxed);
      #pragma omp barrier
      #pragma omp single
      {
        arrsetlen(light_creation_updates, new_light_creation_update_count);
      }

      memcpy(&light_creation_updates[light_creation_index], new_light_creation_updates, arrlenu(new_light_creation_updates) * sizeof *new_light_creation_updates);
      arrfree(new_light_creation_updates);
    }
  }

  if(count != 0)
    LOG_INFO("Processed %zu light creation updates", count);

  profile_end("count", tformat("%zd", count));
}

void light_update(void)
{
  profile_scope;

  process_light_destruction_updates();
  process_light_creation_updates();
}
