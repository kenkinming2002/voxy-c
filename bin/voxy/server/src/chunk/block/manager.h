#ifndef CHUNK_BLOCK_MANAGER_H
#define CHUNK_BLOCK_MANAGER_H

#include <voxy/server/chunk/block/manager.h>

#include <libnet/server.h>

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
bool voxy_block_manager_get_block_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_block_manager_set_block_light_level_atomic(ivec3_t position, uint8_t *light_level, uint8_t *tmp);

void voxy_block_manager_update(void);

void voxy_block_manager_on_client_connected(libnet_client_proxy_t client_proxy);

#endif // CHUNK_BLOCK_MANAGER_H
