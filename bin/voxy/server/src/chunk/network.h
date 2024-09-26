#ifndef CHUNK_NETWORK_H
#define CHUNK_NETWORK_H

#include <libnet/server.h>

#include "chunk.h"

void chunk_network_update(const struct chunk *chunk, libnet_server_t server);
void chunk_network_remove(const struct chunk *chunk, libnet_server_t server);

#endif // CHUNK_NETWORK_H
