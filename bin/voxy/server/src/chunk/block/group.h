#ifndef CHUNK_BLOCK_GROUP_H
#define CHUNK_BLOCK_GROUP_H

#include <voxy/config.h>
#include <voxy/server/chunk/block/group.h>

#include <libmath/vector.h>
#include <libmath/direction.h>

#include <stdint.h>
#include <stdbool.h>

/// Chunk.
struct voxy_block_group
{
  struct voxy_block_group *neighbours[DIRECTION_COUNT];

  /// Blocks.
  ///
  /// We store block ids and light levels in separate arrays for better
  /// "compression".
  uint8_t ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];

  /// If we need to flush this block group to disk.
  bool disk_dirty;

  /// If we need to synchronize this block_group over the network.
  bool network_dirty;
};

/// Create/destroy block group.
///
/// No initialization of any plain old data member is performed.
struct voxy_block_group *voxy_block_group_create(void);
void voxy_block_group_destroy(struct voxy_block_group *block_group);

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
void voxy_block_group_get_block_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_block_group_set_block_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

#endif // CHUNK_BLOCK_GROUP_H
