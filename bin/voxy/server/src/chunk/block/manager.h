#ifndef CHUNK_BLOCK_MANAGER_H
#define CHUNK_BLOCK_MANAGER_H

#include <voxy/server/chunk/block/manager.h>

#include "chunk/manager.h"

#include "group.h"
#include "database.h"
#include "generator.h"

#include <libnet/server.h>

struct voxy_block_registry;
struct voxy_light_manager;

struct voxy_block_manager
{
  struct voxy_block_group_hash_table block_groups;
};

void voxy_block_manager_init(struct voxy_block_manager *block_manager);
void voxy_block_manager_fini(struct voxy_block_manager *block_manager);

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
bool voxy_block_manager_get_block_light_level_atomic(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp);
bool voxy_block_manager_set_block_light_level_atomic(struct voxy_block_manager *block_manager, ivec3_t position, uint8_t *light_level, uint8_t *tmp);

void voxy_block_manager_update(struct voxy_block_manager *block_manager, struct voxy_chunk_manager *chunk_manager, struct voxy_block_database *block_database, struct voxy_block_generator *block_generator, struct voxy_light_manager *light_manager, libnet_server_t server, const struct voxy_context *context);

void voxy_block_manager_on_client_connected(struct voxy_block_manager *block_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_BLOCK_MANAGER_H
