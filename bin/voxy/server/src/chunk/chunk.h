#ifndef CHUNK_CHUNK_H
#define CHUNK_CHUNK_H

#include <voxy/config.h>
#include <voxy/server/chunk/chunk.h>

#include <libmath/vector.h>
#include <libmath/direction.h>

#include <stdint.h>
#include <stdbool.h>

/// Chunk.
struct voxy_chunk
{
  size_t hash;

  struct voxy_chunk *next;
  struct voxy_chunk *neighbours[DIRECTION_COUNT];

  ivec3_t position;

  /// Blocks.
  ///
  /// We store block ids and light levels in separate arrays for better
  /// "compression".
  uint8_t block_ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];

  /// If we need to flush this chunk to disk.
  bool disk_dirty;

  /// If we need to synchronize this chunk over the network.
  bool network_dirty;
};

/// Create/destroy chunk.
///
/// No initialization of any plain old data member is performed.
struct voxy_chunk *voxy_chunk_create(void);
void voxy_chunk_destroy(struct voxy_chunk *chunk);

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
void voxy_chunk_get_block_light_level_atomic(struct voxy_chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_chunk_set_block_light_level_atomic(struct voxy_chunk *chunk, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX voxy_chunk
#define SC_HASH_TABLE_NODE_TYPE struct voxy_chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#endif // CHUNK_CHUNK_H
