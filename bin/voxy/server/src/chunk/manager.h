#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

#include "chunk.h"
#include "generator.h"

#include <libnet/server.h>

struct voxy_block_registry;
struct light_manager;

struct ivec3_node
{
  struct ivec3_node *next;
  size_t hash;
  ivec3_t value;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX ivec3
#define SC_HASH_TABLE_NODE_TYPE struct ivec3_node
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_INTERFACE

struct voxy_chunk_manager
{
  struct ivec3_hash_table active_chunks;
  struct chunk_hash_table chunks;
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
void voxy_chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct chunk_generator *chunk_generator, struct voxy_block_registry *block_registry, struct light_manager *light_manager, libnet_server_t server);

void voxy_chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_MANAGER_H
