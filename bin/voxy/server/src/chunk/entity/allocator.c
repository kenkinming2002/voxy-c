#include "allocator.h"

#include <libcore/log.h>

#include <stb_ds.h>

static struct voxy_entity *entities;
static entity_handle_t *orphans;

entity_handle_t entity_alloc(void)
{
  if(arrlenu(orphans) != 0)
  {
    const entity_handle_t handle = arrpop(orphans);
    entities[handle].alive = true;
    return handle;
  }

  const entity_handle_t handle = arrlenu(entities);
  arrput(entities, (struct voxy_entity){ .alive = true });
  return handle;
}

void entity_free(entity_handle_t handle)
{
  assert(handle < arrlenu(entities));

  struct voxy_entity *entity = &entities[handle];
  struct voxy_entity_info info = voxy_query_entity(entity->id);
  info.destroy_opaque(entity->opaque);

  if(handle == arrlenu(entities) - 1)
  {
    arrpop(entities);
    return;
  }

  entities[handle].alive = false;
  arrput(orphans, handle);
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
