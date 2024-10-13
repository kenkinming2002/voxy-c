#ifndef CHUNK_NETWORK_H
#define CHUNK_NETWORK_H

#include <libnet/server.h>

#include "chunk.h"

void voxy_chunk_network_update(const struct voxy_chunk *chunk, libnet_server_t server);
void voxy_chunk_network_remove(const struct voxy_chunk *chunk, libnet_server_t server);

#endif // CHUNK_NETWORK_H
