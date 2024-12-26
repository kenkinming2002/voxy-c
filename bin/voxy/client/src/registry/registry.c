#include "name.h"

#include <assert.h>

void voxy_name_registry_init(struct voxy_name_registry *registry)
{
  DYNAMIC_ARRAY_INIT(registry->infos);
}

void voxy_name_registry_fini(struct voxy_name_registry *registry)
{
  DYNAMIC_ARRAY_CLEAR(registry->infos);
}

voxy_name_id_t voxy_name_registry_register_name(struct voxy_name_registry *registry, struct voxy_name_info name_info)
{
  const voxy_name_id_t id = registry->infos.item_count;
  DYNAMIC_ARRAY_APPEND(registry->infos, name_info);
  return id;
}

struct voxy_name_info voxy_name_registry_query_name(struct voxy_name_registry *registry, voxy_name_id_t id)
{
  assert(id < registry->infos.item_count);
  return registry->infos.items[id];
}

