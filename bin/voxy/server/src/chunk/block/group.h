#ifndef CHUNK_BLOCK_GROUP_H
#define CHUNK_BLOCK_GROUP_H

#include <voxy/config.h>
#include <voxy/protocol/server.h>
#include <voxy/server/registry/block.h>
#include <voxy/server/chunk/block/group.h>

#include <libmath/vector.h>
#include <libmath/direction.h>

#include <stdint.h>
#include <stdbool.h>

struct voxy_block_group
{
  struct voxy_block_group *neighbours[DIRECTION_COUNT];

  voxy_block_id_t ids[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH];
  uint8_t light_levels[VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH * VOXY_CHUNK_WIDTH / 2];

  bool disk_dirty;
  bool network_dirty;
};

/// Create/destroy a block group.
///
/// The initial content of a newly created block group should be treated as
/// uninitialized.
struct voxy_block_group *voxy_block_group_create(void);
void voxy_block_group_destroy(struct voxy_block_group *block_group);

/// Block getters/setters.
///
/// It is preferable to use voxy_block_group_fill_block_id() over
/// voxy_block_group_set_block_id() if you know that the entire block group
/// consist of blocks of same id. Not only is it stored more efficiently but
/// some part of the game may also be optimized to act on the entire block group
/// as a whole.
voxy_block_id_t voxy_block_group_get_id(const struct voxy_block_group *block_group, ivec3_t position);
void voxy_block_group_set_id(struct voxy_block_group *block_group, ivec3_t position, voxy_block_id_t id);

/// Light level getters/setters.
uint8_t voxy_block_group_get_light_level(const struct voxy_block_group *block_group, ivec3_t position);
void voxy_block_group_set_light_level(struct voxy_block_group *block_group, ivec3_t position, uint8_t light_level);

/// Atomic light level getters/setters.
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
void voxy_block_group_get_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_block_group_set_light_level_atomic(struct voxy_block_group *block_group, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

#endif // CHUNK_BLOCK_GROUP_H
