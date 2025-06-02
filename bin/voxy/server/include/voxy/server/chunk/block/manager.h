#ifndef VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
#define VOXY_SERVER_CHUNK_BLOCK_MANAGER_H

#include <voxy/server/export.h>

#include <libmath/vector.h>

struct voxy_block_manager;
struct voxy_block_registry;
struct voxy_light_manager;

/// Mark a chunk at position as active in current frame.
///
/// We will:
///  - Try to load/generate active chunks.
///  - Try to unload/discard non-active chunks.
///  - Perform updates in active chunks.
VOXY_SERVER_EXPORT void voxy_block_manager_add_active_chunk(struct voxy_block_manager *block_manager, ivec3_t position);

/// FIXME: Figure out a better API so that we no longer have to perform repeated
///        chunk lookup each time we call a getter or setter.

/// Getters.
///
/// In case the chunk does not exist, value in def is returned.
VOXY_SERVER_EXPORT uint8_t voxy_block_manager_get_block_id(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t def);
VOXY_SERVER_EXPORT uint8_t voxy_block_manager_get_block_light_level(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t def);

/// Setters.
VOXY_SERVER_EXPORT void voxy_block_manager_set_block_id(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t id);
VOXY_SERVER_EXPORT void voxy_block_manager_set_block_light_level(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t light_level);

/// Set block at given position.
///
/// The light level of the block will be derived from light info in block
/// registry. This will also enqueue any necessary light updates to light
/// manager..
VOXY_SERVER_EXPORT void voxy_block_manager_set_block(
    struct voxy_block_manager *block_manager,
    struct voxy_light_manager *light_manager,
    ivec3_t position,
    uint8_t id);

#endif // VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
