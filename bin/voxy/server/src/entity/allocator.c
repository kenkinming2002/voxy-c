#include "allocator.h"

#include <libcommon/core/log.h>

void entity_allocator_init(struct entity_allocator *entity_manager)
{
  DYNAMIC_ARRAY_INIT(entity_manager->entities);
  DYNAMIC_ARRAY_INIT(entity_manager->orphans);
}

void entity_allocator_fini(struct entity_allocator *entity_manager)
{
  DYNAMIC_ARRAY_CLEAR(entity_manager->orphans);
  DYNAMIC_ARRAY_CLEAR(entity_manager->entities);
}

entity_handle_t entity_allocator_alloc(struct entity_allocator *entity_manager)
{
  if(entity_manager->orphans.item_count != 0)
  {
    const entity_handle_t handle = entity_manager->orphans.items[--entity_manager->orphans.item_count];
    entity_manager->entities.items[handle].alive = true;
    return handle;
  }

  const entity_handle_t handle = entity_manager->entities.item_count;
  DYNAMIC_ARRAY_APPEND(entity_manager->entities, (struct voxy_entity){ .alive = true });
  return handle;
}

void entity_allocator_free(struct entity_allocator *entity_manager, entity_handle_t handle)
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

struct voxy_entity *entity_allocator_get(struct entity_allocator *entity_allocator, entity_handle_t handle)
{
  assert(handle < entity_allocator->entities.item_count);
  return &entity_allocator->entities.items[handle];
}

