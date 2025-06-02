#ifndef CHUNK_ENTITY_MANAGER_H
#define CHUNK_ENTITY_MANAGER_H

#include "entity.h"

#include <libnet/client.h>

#include <stdint.h>

typedef uint32_t entity_handle_t;
#define ENTITY_HANDLE_NULL UINT32_MAX

void entity_manager_on_message_received(libnet_client_t client, const struct libnet_message *message);

void entity_manager_update_entity(entity_handle_t handle, voxy_entity_id_t id,  fvec3_t position, fvec3_t rotation);
void entity_manager_remove_entity(entity_handle_t handle);

struct voxy_entity *entity_get(entity_handle_t handle);
struct voxy_entity *entity_get_all(void);

#endif // CHUNK_ENTITY_MANAGER_H
