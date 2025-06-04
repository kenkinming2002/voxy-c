#ifndef CHUNK_BLOCK_GROUP_H
#define CHUNK_BLOCK_GROUP_H

#include <voxy/config.h>
#include <voxy/types.h>

#include <libmath/vector.h>
#include <libmath/direction.h>

#include <stdint.h>

/// Chunk.
struct block_group
{
  struct block_group *neighbours[DIRECTION_COUNT];

  voxy_block_id_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  voxy_light_t block_lights[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];

  bool remesh;
};

/// Create/destroy block group.
///
/// No initialization of any plain old data member is performed.
struct block_group *block_group_create(void);
void block_group_destroy(struct block_group *block_group);

/// Accessors.
uint8_t block_group_get_block_id(const struct block_group *block_group, ivec3_t position);
voxy_light_t block_group_get_block_light(const struct block_group *block_group, ivec3_t position);

/// Extended Accessors.
///
/// This also walk to neighbouring block groups. In case neighbouring block groups does not
/// exist, value provided in def is returned.
uint8_t block_group_get_block_id_ex(const struct block_group *block_group, ivec3_t position, uint8_t def);
voxy_light_t block_group_get_block_light_ex(const struct block_group *block_group, ivec3_t position, voxy_light_t def);

#endif // CHUNK_BLOCK_GROUP_H
