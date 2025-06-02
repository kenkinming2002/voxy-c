#ifndef CHUNK_BLOCK_MANAGER_H
#define CHUNK_BLOCK_MANAGER_H

#include "group.h"

#include <libnet/client.h>

struct block_group *get_block_group(ivec3_t chunk_position);
struct block_group *get_or_insert_block_group(ivec3_t chunk_position);
void block_manager_remove_block_group(ivec3_t chunk_position);

void block_manager_on_message_received(const struct libnet_message *message);

#endif // CHUNK_BLOCK_MANAGER_H
