#ifndef CHUNK_CHUNK_H
#define CHUNK_CHUNK_H

#include <voxy/protocol/chunk.h>

#include <libcommon/math/vector.h>
#include <libcommon/math/direction.h>

#include <stdint.h>

/// Chunk.
struct chunk
{
  size_t hash;

  struct chunk *next;
  struct chunk *neighbours[DIRECTION_COUNT];

  ivec3_t position;
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];
};

/// Create/destroy chunk.
///
/// No initialization of any plain old data member is performed.
struct chunk *chunk_create(void);
void chunk_destroy(struct chunk *chunk);

/// Accessors.
uint8_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position);
uint8_t chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position);

/// Extended Accessors.
///
/// This also walk to neighbouring chunks. In case neighbouring chunk does not
/// exist, value provided in def is returned.
uint8_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position, uint8_t def);
uint8_t chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position, uint8_t def);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE


#endif // CHUNK_CHUNK_H
