#include "manager.h"

#include <assert.h>

void entity_manager_init(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_INIT(entity_manager->entities);
  DYNAMIC_ARRAY_INIT(entity_manager->orphans);
}

void entity_manager_fini(struct entity_manager *entity_manager)
{
  DYNAMIC_ARRAY_CLEAR(entity_manager->orphans);
  DYNAMIC_ARRAY_CLEAR(entity_manager->entities);
}

entity_handle_t entity_manager_alloc(struct entity_manager *entity_manager)
{
  if(entity_manager->orphans.item_count != 0)
    return entity_manager->orphans.items[entity_manager->orphans.item_count--];

  const entity_handle_t handle = entity_manager->entities.item_count;
  DYNAMIC_ARRAY_RESERVE(entity_manager->entities, entity_manager->entities.item_count+1);
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
  DYNAMIC_ARRAY_APPEND(entity_manager->orphans, handle);
}

struct entity *entity_manager_get(struct entity_manager *entity_manager, entity_handle_t handle)
{
  assert(handle < entity_manager->entities.item_count);
  return &entity_manager->entities.items[handle];
}

