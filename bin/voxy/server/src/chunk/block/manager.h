#ifndef CHUNK_BLOCK_MANAGER_H
#define CHUNK_BLOCK_MANAGER_H

#include <voxy/server/chunk/block/manager.h>

#include <libnet/server.h>

/// Get the block group at given chunk position.
///
/// Returns NULL if the block group does not exist.
struct voxy_block_group *voxy_get_block_group(ivec3_t chunk_position);

/// Block id/light level setters.
///
/// In case the block dees not exist (for example if the chunk in which the
/// block resides in has not yet been loaded/generated), return false.
bool voxy_set_block_id(ivec3_t position, voxy_block_id_t id);
bool voxy_set_block_light_level(ivec3_t position, uint8_t light_level);

/// Atomic block light level getter/setter.
///
/// What is so hard about atomicity you may ask? The problem we have is that
/// light level for a block is technically a uint4_t but computers does not work
/// at bit granularity.
///
/// This means the best that we could do is to *simulate* atomicity using
/// the cmpxchg instruction.
///
/// Hence, the following interfaces.
bool voxy_get_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_set_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp);

void voxy_block_manager_update(void);
void voxy_block_manager_on_client_connected(libnet_client_proxy_t client_proxy);

#endif // CHUNK_BLOCK_MANAGER_H
