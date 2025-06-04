#include "group.h"

#include <voxy/config.h>
#include <voxy/server/registry/block.h>

#include <libcore/unreachable.h>

#include <limits.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <string.h>
#include <stdlib.h>

static void check_position(ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);
}

struct voxy_block_group *voxy_block_group_create(void)
{
  struct voxy_block_group *block_group = malloc(sizeof *block_group);
  return block_group;
}

void voxy_block_group_destroy(struct voxy_block_group *block_group)
{
  free(block_group);
}

uint8_t voxy_block_group_get_id(const struct voxy_block_group *block_group, ivec3_t position)
{
  check_position(position);
  return block_group->ids[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x];
}

void voxy_block_group_set_id(struct voxy_block_group *block_group, ivec3_t position, uint8_t id)
{
  check_position(position);
  block_group->ids[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x] = id;
  block_group->disk_dirty = true;
  block_group->network_dirty = true;
}

voxy_light_t voxy_block_group_get_light(const struct voxy_block_group *block_group, ivec3_t position)
{
  check_position(position);
  return block_group->lights[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x];
}

void voxy_block_group_set_light(struct voxy_block_group *block_group, ivec3_t position, voxy_light_t light)
{
  check_position(position);
  block_group->lights[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x] = light;
  block_group->disk_dirty = true;
  block_group->network_dirty = true;
}

voxy_light_t voxy_block_group_get_light_atomic(const struct voxy_block_group *block_group, ivec3_t position)
{
  check_position(position);
  return *(voxy_light_t *)&(uint8_t){atomic_load((_Atomic uint8_t *)&block_group->lights[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x])};
}

void voxy_block_group_set_light_atomic(struct voxy_block_group *block_group, ivec3_t position, voxy_light_t light)
{
  static_assert(alignof(_Atomic uint8_t) == alignof(voxy_light_t), "");
  static_assert(sizeof(_Atomic uint8_t) == sizeof(voxy_light_t), "");

  check_position(position);
  atomic_store((_Atomic uint8_t *)&block_group->lights[position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x], *(const uint8_t *)&light);
  atomic_store((_Atomic bool *)&block_group->disk_dirty, true);
  atomic_store((_Atomic bool *)&block_group->network_dirty, true);
}

