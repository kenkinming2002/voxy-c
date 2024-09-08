#ifndef CHUNK_CHUNK_H
#define CHUNK_CHUNK_H

#include <voxy/protocol/chunk.h>

#include <libcommon/math/vector.h>
#include <libcommon/math/direction.h>

#include <stdint.h>
#include <stdbool.h>

/// Chunk.
struct chunk
{
  size_t        hash;
  struct chunk *next;

  ivec3_t position;
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];

  bool dirty;
};

/// Create/destroy chunk.
///
/// No initialization of any plain old data member is performed.
struct chunk *chunk_create(void);
void chunk_destroy(struct chunk *chunk);

/// Getters.
uint8_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position);
uint8_t chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position);

/// Setters.
void chunk_set_block_id(struct chunk *chunk, ivec3_t position, uint8_t id);
void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, uint8_t light_level);

/// Atomic getters/setters.
///
/// What is so hard about atomicity you may ask? The problem we have is that
/// light level for a block is technically a uint4_t but computers does not work
/// at bit granularity.
///
/// This means the best that we could do is to *simulate* atomicity using
/// the cmpxchg instruction.
///
/// Hence, the following interfaces.
///
/// Note: This need not be exposed to mod, since it is really only used in the
///       implementation of our light system.
void chunk_get_block_light_level_atomic(struct chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

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
