#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <libnet/server.h>

struct chunk_manager
{
};

int chunk_manager_init(struct chunk_manager *chunk_manager);
void chunk_manager_fini(struct chunk_manager *chunk_manager);

void chunk_manager_update(struct chunk_manager *chunk_manager);
void chunk_manager_on_client_connected(struct chunk_manager *chunk_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // CHUNK_MANAGER_H
