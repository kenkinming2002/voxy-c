#include "group.h"

#include <voxy/config.h>
#include <voxy/server/registry/block.h>

#include <libcore/unreachable.h>

#include <limits.h>
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

uint8_t voxy_block_group_get_light_level(const struct voxy_block_group *block_group, ivec3_t position)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const unsigned q = index / 2;
  const unsigned r = index % 2;
  return (block_group->light_levels[q] >> (4 * r)) & 0xF;
}

void voxy_block_group_set_light_level(struct voxy_block_group *block_group, ivec3_t position, uint8_t light_level)
{
  check_position(position);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const unsigned q = index / 2;
  const unsigned r = index % 2;
  block_group->light_levels[q] &= ~(((1 << 4) - 1) << (r * 4));
  block_group->light_levels[q] |= light_level << (r * 4);

  block_group->disk_dirty = true;
  block_group->network_dirty = true;
}

void voxy_block_group_get_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  check_position(position);

  const size_t offset = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  *tmp = atomic_load((_Atomic unsigned char *)&block_group->light_levels[q]);
  *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
}

bool voxy_block_group_set_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp)
{
  check_position(position);

  const size_t offset = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  const size_t q = offset / (CHAR_BIT / 4);
  const size_t r = offset % (CHAR_BIT / 4);

  unsigned char desired = *tmp;
  desired &= ~(((1 << 4) - 1) << (r * 4));
  desired |= *light_level << (r * 4);
  if(!atomic_compare_exchange_strong((_Atomic unsigned char *)&block_group->light_levels[q], tmp, desired))
  {
    *light_level = (*tmp >> (r * 4)) & ((1 << 4) - 1);
    return false;
  }

  atomic_store((_Atomic bool *)&block_group->disk_dirty, true);
  atomic_store((_Atomic bool *)&block_group->network_dirty, true);
  return true;
}

