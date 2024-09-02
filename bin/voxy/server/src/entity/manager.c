#include "manager.h"

#include <voxy/protocol/server.h>

#include <assert.h>

void entity_manager_init(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_INIT(entity_manager->entities);
  DYNAMIC_ARRAY_INIT(entity_manager->orphans);

  for(unsigned i=0; i<100; ++i)
  {
    struct entity *entity = entity_manager_get(entity_manager, entity_manager_alloc(entity_manager));
    entity->id = i % 2;
    entity->position = fvec3(i, i, i);
    entity->rotation = fvec3(i, i, i);
  }
}

void entity_manager_fini(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_CLEAR(entity_manager->orphans);
  DYNAMIC_ARRAY_CLEAR(entity_manager->entities);
}

void entity_manager_update(struct entity_manager *entity_manager)
{
  (void)entity_manager;
}

void entity_manager_on_client_connected(struct entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  for(entity_handle_t handle=0; handle<entity_manager->entities.item_count; ++handle)
  {
    const struct entity *entity = &entity_manager->entities.items[handle];
    if(entity_manager->entities.items[handle].alive)
    {
      struct voxy_server_entity_update_message message;
      message.message.message.size = LIBNET_MESSAGE_SIZE(message);
      message.message.tag = VOXY_SERVER_MESSAGE_ENTITY_UPDATE;
      message.handle = handle;
      message.id = entity->id;
      message.position = entity->position;
      message.rotation = entity->rotation;
      libnet_server_send_message(server, client_proxy, &message.message.message);
    }
  }
}

entity_handle_t entity_manager_alloc(struct entity_manager *entity_manager)
{
  if(entity_manager->orphans.item_count != 0)
    return entity_manager->orphans.items[entity_manager->orphans.item_count--];

  const entity_handle_t handle = entity_manager->entities.item_count;
  DYNAMIC_ARRAY_APPEND(entity_manager->entities, (struct entity){ .alive = true });
  return handle;
}

void entity_manager_free(struct entity_manager *entity_manager, entity_handle_t handle)
{
  assert(handle < entity_manager->entities.item_count);

  if(handle == entity_manager->entities.item_count - 1)
  {
    --entity_manager->entities.item_count;
    return;
  }

  entity_manager->entities.items[handle].alive = false;
  DYNAMIC_ARRAY_APPEND(entity_manager->orphans, handle);
}

struct entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle)
{
  assert(handle < entity_manager->entities.item_count);
  return &entity_manager->entities.items[handle];
}

