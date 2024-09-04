#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

#include "chunk.h"
#include "generator.h"

#include <libnet/server.h>

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

void chunk_manager_init(struct voxy_chunk_manager *chunk_manager);
void chunk_manager_fini(struct voxy_chunk_manager *chunk_manager);

void chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager);
void chunk_manager_update(struct voxy_chunk_manager *chunk_manager, struct chunk_generator *chunk_generator, libnet_server_t server);

void chunk_manager_on_client_connected(struct voxy_chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_MANAGER_H
