#ifndef CHUNK_BLOCK_NETWORK_H
#define CHUNK_BLOCK_NETWORK_H

#include <libnet/server.h>

#include "group.h"

void voxy_block_network_update(const struct voxy_block_group *block_group, libnet_server_t server);
void voxy_block_network_remove(const struct voxy_block_group *block_group, libnet_server_t server);

#endif // CHUNK_BLOCK_NETWORK_H
