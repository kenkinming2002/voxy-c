#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

#include "chunk.h"
#include "generator.h"

#include "hash_table/ivec3.h"

#include <libnet/server.h>

struct voxy_block_registry;
struct voxy_light_manager;

struct voxy_chunk_manager
{
  struct ivec3_hash_table active_chunks;
  struct voxy_chunk_hash_table chunks;
};

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager);
void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager);

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
bool voxy_chunk_manager_get_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_chunk_manager_set_block_light_level_atomic(struct voxy_chunk_manager *chunk_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

void voxy_chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager);
void voxy_chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct voxy_chunk_generator *chunk_generator, struct voxy_light_manager *light_manager, libnet_server_t server, const struct voxy_context *context);

void voxy_chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_MANAGER_H
