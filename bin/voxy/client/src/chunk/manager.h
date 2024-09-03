#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include "chunk.h"

#include <libnet/client.h>

struct chunk_manager
{
  struct chunk_hash_table chunks;
};

int chunk_manager_init(struct chunk_manager *chunk_manager);
void chunk_manager_fini(struct chunk_manager *chunk_manager);

struct chunk *chunk_manager_get_chunk(struct chunk_manager *chunk_manager, ivec3_t position);
void chunk_manager_remove_chunk(struct chunk_manager *chunk_manager, ivec3_t position);

void chunk_manager_update(struct chunk_manager *chunk_manager);
void chunk_manager_on_message_received(struct chunk_manager *chunk_manager, libnet_client_t client, const struct libnet_message *message);

#endif // CHUNK_MANAGER_H
