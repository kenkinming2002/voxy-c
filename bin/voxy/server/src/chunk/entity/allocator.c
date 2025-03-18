#include "allocator.h"

#include <libcore/log.h>

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

void entity_allocator_free(struct entity_allocator *entity_allocator, struct voxy_entity_registry *entity_registry, entity_handle_t handle)
{
  assert(handle < entity_allocator->entities.item_count);

  struct voxy_entity *entity = &entity_allocator->entities.items[handle];
  struct voxy_entity_info info = voxy_entity_registry_query_entity(entity_registry, entity->id);
  info.destroy_opaque(entity->opaque);

  if(handle == entity_allocator->entities.item_count - 1)
  {
    --entity_allocator->entities.item_count;
    return;
  }

  entity_allocator->entities.items[handle].alive = false;
  DYNAMIC_ARRAY_APPEND(entity_allocator->orphans, handle);
}

struct voxy_entity *entity_allocator_get(struct entity_allocator *entity_allocator, entity_handle_t handle)
{
  assert(handle < entity_allocator->entities.item_count);
  return &entity_allocator->entities.items[handle];
}

