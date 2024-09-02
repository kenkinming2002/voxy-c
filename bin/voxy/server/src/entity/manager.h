#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "entity.h"

#include <libnet/server.h>

#include <stdint.h>

typedef uint32_t entity_handle_t;

struct entity_manager
{
  DYNAMIC_ARRAY_DEFINE(,struct entity) entities;
  DYNAMIC_ARRAY_DEFINE(,entity_handle_t) orphans;
};

void entity_manager_init(struct entity_manager *entity_manager);
void entity_manager_fini(struct entity_manager *entity_manager);

void entity_manager_update(struct entity_manager *entity_manager);
void entity_manager_on_client_connected(struct entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

entity_handle_t entity_manager_alloc(struct entity_manager *entity_manager);
void entity_manager_free(struct entity_manager *entity_manager, entity_handle_t handle);

struct entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle);

#endif // ENTITY_MANAGER_H
