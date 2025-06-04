#include "group.h"

#include <voxy/config.h>

#include <stdlib.h>

struct block_group *block_group_create(void)
{
  struct block_group *block_group = malloc(sizeof *block_group);
  return block_group;
}

void block_group_destroy(struct block_group *block_group)
{
  free(block_group);
}

uint8_t block_group_get_block_id(const struct block_group *block_group, ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  return block_group->block_ids[index];
}

voxy_light_t block_group_get_block_light(const struct block_group *block_group, ivec3_t position)
{
  assert(0 <= position.x && position.x < VOXY_CHUNK_WIDTH);
  assert(0 <= position.y && position.y < VOXY_CHUNK_WIDTH);
  assert(0 <= position.z && position.z < VOXY_CHUNK_WIDTH);

  const unsigned index = position.z * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH + position.y * VOXY_CHUNK_WIDTH + position.x;
  return block_group->block_lights[index];
}

static void traverse(const struct block_group **block_group, ivec3_t *position)
{
  while(*block_group && position->x < 0) { (*block_group) = (*block_group)->neighbours[DIRECTION_LEFT];   position->x += VOXY_CHUNK_WIDTH; }
  while(*block_group && position->y < 0) { (*block_group) = (*block_group)->neighbours[DIRECTION_BACK];   position->y += VOXY_CHUNK_WIDTH; }
  while(*block_group && position->z < 0) { (*block_group) = (*block_group)->neighbours[DIRECTION_BOTTOM]; position->z += VOXY_CHUNK_WIDTH; }

  while(*block_group && position->x >= VOXY_CHUNK_WIDTH) { (*block_group) = (*block_group)->neighbours[DIRECTION_RIGHT]; position->x -= VOXY_CHUNK_WIDTH; }
  while(*block_group && position->y >= VOXY_CHUNK_WIDTH) { (*block_group) = (*block_group)->neighbours[DIRECTION_FRONT]; position->y -= VOXY_CHUNK_WIDTH; }
  while(*block_group && position->z >= VOXY_CHUNK_WIDTH) { (*block_group) = (*block_group)->neighbours[DIRECTION_TOP];   position->z -= VOXY_CHUNK_WIDTH; }
}

uint8_t block_group_get_block_id_ex(const struct block_group *block_group, ivec3_t position, uint8_t def)
{
  traverse(&block_group, &position);
  return block_group ? block_group_get_block_id(block_group, position) : def;
}

voxy_light_t block_group_get_block_light_ex(const struct block_group *block_group, ivec3_t position, voxy_light_t def)
{
  traverse(&block_group, &position);
  return block_group ? block_group_get_block_light(block_group, position) : def;
}

