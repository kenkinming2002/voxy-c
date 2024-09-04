#include "registry.h"

#include <assert.h>

void voxy_entity_registry_init(struct voxy_entity_registry *registry)
{
  DYNAMIC_ARRAY_INIT(registry->infos);
}

void voxy_entity_registry_fini(struct voxy_entity_registry *registry)
{
  DYNAMIC_ARRAY_CLEAR(registry->infos);
}

entity_id_t voxy_entity_registry_register_entity(struct voxy_entity_registry *registry, struct voxy_entity_info entity_info)
{
  const entity_id_t id = registry->infos.item_count;
  DYNAMIC_ARRAY_APPEND(registry->infos, entity_info);
  return id;
}

struct voxy_entity_info voxy_entity_registry_query_entity(struct voxy_entity_registry *registry, entity_id_t id)
{
  assert(id < registry->infos.item_count);
  return registry->infos.items[id];
}


