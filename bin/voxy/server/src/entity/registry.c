#include "registry.h"

#include <assert.h>

void entity_registry_init(struct entity_registry *registry)
{
  DYNAMIC_ARRAY_INIT(registry->infos);
}

void entity_registry_fini(struct entity_registry *registry)
{
  DYNAMIC_ARRAY_CLEAR(registry->infos);
}

entity_id_t entity_registry_register_entity(struct entity_registry *registry, struct entity_info entity_info)
{
  const entity_id_t id = registry->infos.item_count;
  DYNAMIC_ARRAY_APPEND(registry->infos, entity_info);
  return id;
}

struct entity_info entity_registry_query_entity(struct entity_registry *registry, entity_id_t id)
{
  assert(id < registry->infos.item_count);
  return registry->infos.items[id];
}


