#include "manager.h"

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

void entity_manager_on_message_received(struct entity_manager *entity_manager, libnet_client_t client, const struct libnet_message *message)
{
  (void)entity_manager;
  (void)client;
  (void)message;
}

struct entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle)
{
  assert(handle < entity_manager->entities.item_count);
  return &entity_manager->entities.items[handle];
}

