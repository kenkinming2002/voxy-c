#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>

#include <assert.h>

void entity_manager_init(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_INIT(entity_manager->entities);
}

void entity_manager_fini(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_CLEAR(entity_manager->entities);
}

void entity_manager_update(struct entity_manager *entity_manager)
{
  (void)entity_manager;
}

void entity_manager_on_message_received(struct entity_manager *entity_manager, libnet_client_t client, const struct libnet_message *_message)
{
  (void)client;

  {
    struct voxy_server_entity_update_message *message = voxy_get_server_entity_update_message(_message);
    if(message)
      entity_manager_update_entity(entity_manager, message->handle, message->id, message->position, message->rotation);
  }

  {
    struct voxy_server_entity_remove_message *message = voxy_get_server_entity_remove_message(_message);
    if(message)
      entity_manager_remove_entity(entity_manager, message->handle);
  }
}

struct voxy_entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle)
{
  assert(handle < entity_manager->entities.item_count);
  return &entity_manager->entities.items[handle];
}

void entity_manager_update_entity(struct entity_manager *entity_manager, entity_handle_t handle, voxy_entity_id_t id,  fvec3_t position, fvec3_t rotation)
{
  // FIXME: Maybe we need some protection against malicious server allocating a
  //        high entity handle.
  // FIXME: Potential integer overflow.
  DYNAMIC_ARRAY_RESERVE(entity_manager->entities, handle+1);
  while(entity_manager->entities.item_count <= handle)
    entity_manager->entities.items[entity_manager->entities.item_count++].alive = false;

  entity_manager->entities.items[handle].alive = true;
  entity_manager->entities.items[handle].id = id;
  entity_manager->entities.items[handle].position = position;
  entity_manager->entities.items[handle].rotation = rotation;
}

void entity_manager_remove_entity(struct entity_manager *entity_manager, entity_handle_t handle)
{
  if(entity_manager->entities.item_count <= handle)
  {
    LOG_ERROR("Attempting to remove entity with handle %u but number of entities in the pool is only %zu. Ignoring...", handle, entity_manager->entities.item_count);
    return;
  }

  if(!entity_manager->entities.items[handle].alive)
  {
    LOG_ERROR("Attempting to remove entity with handle %u which is not alive. Ignoring...", handle);
    return;
  }

  entity_manager->entities.items[handle].alive = false;
}
