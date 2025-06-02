#ifndef CHUNK_BLOCK_NETWORK_H
#define CHUNK_BLOCK_NETWORK_H

#include <libnet/server.h>

#include "group.h"

void voxy_block_network_update(ivec3_t position, const struct voxy_block_group *block_group);
void voxy_block_network_remove(ivec3_t position);

#endif // CHUNK_BLOCK_NETWORK_H
