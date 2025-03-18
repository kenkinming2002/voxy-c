#ifndef VOXY_SERVER_CHUNK_BLOCK_GROUP_H
#define VOXY_SERVER_CHUNK_BLOCK_GROUP_H

#include <voxy/server/export.h>
#include <libmath/vector.h>

struct voxy_block_group;
struct voxy_block_registry;

/// Getters.
VOXY_SERVER_EXPORT uint8_t voxy_block_group_get_block_id(const struct voxy_block_group *block_group, ivec3_t position);
VOXY_SERVER_EXPORT uint8_t voxy_block_group_get_block_light_level(const struct voxy_block_group *block_group, ivec3_t position);

/// Setters.
VOXY_SERVER_EXPORT void voxy_block_group_set_block_id(struct voxy_block_group *block_group, ivec3_t position, uint8_t id);
VOXY_SERVER_EXPORT void voxy_block_group_set_block_light_level(struct voxy_block_group *block_group, ivec3_t position, uint8_t light_level);

/// Set block at given position.
///
/// The light level of the block will be derived from light info in block
/// registry.
VOXY_SERVER_EXPORT void voxy_block_group_set_block(struct voxy_block_group *block_group, struct voxy_block_registry *block_registry, ivec3_t position, uint8_t id);

#endif // VOXY_SERVER_CHUNK_BLOCK_GROUP_H
