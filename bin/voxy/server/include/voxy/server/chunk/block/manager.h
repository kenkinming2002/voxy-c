#ifndef VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
#define VOXY_SERVER_CHUNK_BLOCK_MANAGER_H

#include <voxy/server/export.h>

#include <libmath/vector.h>

struct voxy_block_group;

/// Get the block group at given chunk position.
///
/// Returns NULL if the block group does not exist.
VOXY_SERVER_EXPORT struct voxy_block_group *voxy_get_block_group(ivec3_t chunk_position);

/// FIXME: Figure out a better API so that we no longer have to perform repeated
///        chunk lookup each time we call a getter or setter.

/// Getters.
///
/// In case the chunk does not exist, value in def is returned.
VOXY_SERVER_EXPORT uint8_t voxy_get_block_id(ivec3_t position, uint8_t def);
VOXY_SERVER_EXPORT uint8_t voxy_get_block_light_level(ivec3_t position, uint8_t def);

/// Setters.
VOXY_SERVER_EXPORT void voxy_set_block_id(ivec3_t position, uint8_t id);
VOXY_SERVER_EXPORT void voxy_set_block_light_level(ivec3_t position, uint8_t light_level);

/// Set block at given position.
///
/// The light level of the block will be derived from light info in block
/// registry. This will also enqueue any necessary light updates to light
/// manager..
VOXY_SERVER_EXPORT void voxy_set_block(ivec3_t position, uint8_t id);

#endif // VOXY_SERVER_CHUNK_BLOCK_MANAGER_H
