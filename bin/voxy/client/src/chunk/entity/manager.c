#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcore/log.h>

#include <stb_ds.h>

#include <assert.h>

static struct voxy_entity *entities;

void entity_manager_on_message_received(const struct libnet_message *_message)
{
  {
    struct voxy_server_entity_update_message *message = voxy_get_server_entity_update_message(_message);
    if(message)
      entity_manager_update_entity(message->handle, message->id, message->position, message->rotation);
  }

  {
    struct voxy_server_entity_remove_message *message = voxy_get_server_entity_remove_message(_message);
    if(message)
      entity_manager_remove_entity(message->handle);
  }
}

struct voxy_entity *entity_get(entity_handle_t handle)
{
  assert(handle < arrlenu(entities));
  return &entities[handle];
}

struct voxy_entity *entity_get_all(void)
{
  return entities;
}

void entity_manager_update_entity(entity_handle_t handle, voxy_entity_id_t id,  fvec3_t position, fvec3_t rotation)
{
  // FIXME: Maybe we need some protection against malicious server allocating a
  //        high entity handle.
  // FIXME: Potential integer overflow.
  size_t old_len = arrlenu(entities);
  arrsetlen(entities, handle+1);
  for(size_t i=old_len; i<arrlenu(entities); ++i)
    entities[i].alive = false;

  entities[handle].alive = true;
  entities[handle].id = id;
  entities[handle].position = position;
  entities[handle].rotation = rotation;
}

void entity_manager_remove_entity(entity_handle_t handle)
{
  if(arrlenu(entities) <= handle)
  {
    LOG_ERROR("Attempting to remove entity with handle %u but number of entities in the pool is only %zu. Ignoring...", handle, arrlenu(entities));
    return;
  }

  if(!entities[handle].alive)
  {
    LOG_ERROR("Attempting to remove entity with handle %u which is not alive. Ignoring...", handle);
    return;
  }

  entities[handle].alive = false;
}
