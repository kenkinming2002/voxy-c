#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include "entity.h"

#include <libnet/client.h>

#include <stdint.h>

typedef uint32_t entity_handle_t;
#define ENTITY_HANDLE_NULL UINT32_MAX

struct entity_manager
{
  DYNAMIC_ARRAY_DEFINE(,struct voxy_entity) entities;
};

void entity_manager_init(struct entity_manager *entity_manager);
void entity_manager_fini(struct entity_manager *entity_manager);

void entity_manager_update(struct entity_manager *entity_manager);
void entity_manager_on_message_received(struct entity_manager *entity_manager, libnet_client_t client, const struct libnet_message *message);

struct voxy_entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle);

void entity_manager_update_entity(struct entity_manager *entity_manager, entity_handle_t handle, voxy_entity_id_t id,  fvec3_t position, fvec3_t rotation);
void entity_manager_remove_entity(struct entity_manager *entity_manager, entity_handle_t handle);

#endif // ENTITY_MANAGER_H
