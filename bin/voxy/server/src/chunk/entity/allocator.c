#include "allocator.h"

#include <libcore/log.h>

#include <stb_ds.h>

void entity_allocator_init(struct entity_allocator *entity_manager)
{
  entity_manager->entities = NULL;
  entity_manager->orphans = NULL;
}

void entity_allocator_fini(struct entity_allocator *entity_manager)
{
  arrfree(entity_manager->orphans);
  arrfree(entity_manager->entities);
}

entity_handle_t entity_allocator_alloc(struct entity_allocator *entity_manager)
{
  if(arrlenu(entity_manager->orphans) != 0)
  {
    const entity_handle_t handle = arrpop(entity_manager->orphans);
    entity_manager->entities[handle].alive = true;
    return handle;
  }

  const entity_handle_t handle = arrlenu(entity_manager->entities);
  arrput(entity_manager->entities, (struct voxy_entity){ .alive = true });
  return handle;
}

void entity_allocator_free(struct entity_allocator *entity_allocator, struct voxy_entity_registry *entity_registry, entity_handle_t handle)
{
  assert(handle < arrlenu(entity_allocator->entities));

  struct voxy_entity *entity = &entity_allocator->entities[handle];
  struct voxy_entity_info info = voxy_entity_registry_query_entity(entity_registry, entity->id);
  info.destroy_opaque(entity->opaque);

  if(handle == arrlenu(entity_allocator->entities) - 1)
  {
    arrpop(entity_allocator->entities);
    return;
  }

  entity_allocator->entities[handle].alive = false;
  arrput(entity_allocator->orphans, handle);
}

struct voxy_entity *entity_allocator_get(struct entity_allocator *entity_allocator, entity_handle_t handle)
{
  assert(handle < arrlenu(entity_allocator->entities));
  return &entity_allocator->entities[handle];
}

