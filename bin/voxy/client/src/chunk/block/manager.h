#ifndef CHUNK_BLOCK_MANAGER_H
#define CHUNK_BLOCK_MANAGER_H

#include "group.h"

#include <libnet/client.h>

struct block_group_node
{
  ivec3_t key;
  struct block_group *value;
};


struct block_manager
{
  struct block_group_node *block_group_nodes;
};

int block_manager_init(struct block_manager *block_manager);
void block_manager_fini(struct block_manager *block_manager);

struct block_group *block_manager_get_block_group(struct block_manager *block_manager, ivec3_t position);
void block_manager_remove_block_group(struct block_manager *block_manager, ivec3_t position);

void block_manager_update(struct block_manager *block_manager);
void block_manager_on_message_received(struct block_manager *block_manager, libnet_client_t client, const struct libnet_message *message);

#endif // CHUNK_BLOCK_MANAGER_H
