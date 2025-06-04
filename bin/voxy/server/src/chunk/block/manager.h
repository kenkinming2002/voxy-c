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
bool voxy_set_block_light(ivec3_t position, voxy_light_t light);

void voxy_block_manager_update(void);
void voxy_block_manager_on_client_connected(libnet_client_proxy_t client_proxy);

#endif // CHUNK_BLOCK_MANAGER_H
