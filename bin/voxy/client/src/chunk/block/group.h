#ifndef CHUNK_BLOCK_GROUP_H
#define CHUNK_BLOCK_GROUP_H

#include <voxy/config.h>

#include <libmath/vector.h>
#include <libmath/direction.h>

#include <stdint.h>

/// Chunk.
struct block_group
{
  size_t hash;

  struct block_group *next;
  struct block_group *neighbours[DIRECTION_COUNT];

  ivec3_t position;
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];

  bool remesh;
};

/// Create/destroy block group.
///
/// No initialization of any plain old data member is performed.
struct block_group *block_group_create(void);
void block_group_destroy(struct block_group *block_group);

/// Accessors.
uint8_t block_group_get_block_id(const struct block_group *block_group, ivec3_t position);
uint8_t block_group_get_block_light_level(const struct block_group *block_group, ivec3_t position);

/// Extended Accessors.
///
/// This also walk to neighbouring block groups. In case neighbouring block groups does not
/// exist, value provided in def is returned.
uint8_t block_group_get_block_id_ex(const struct block_group *block_group, ivec3_t position, uint8_t def);
uint8_t block_group_get_block_light_level_ex(const struct block_group *block_group, ivec3_t position, uint8_t def);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX block_group
#define SC_HASH_TABLE_NODE_TYPE struct block_group
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE


#endif // CHUNK_BLOCK_GROUP_H
