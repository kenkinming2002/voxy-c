#include "name.h"

#include <stb_ds.h>

#include <assert.h>

void voxy_name_registry_init(struct voxy_name_registry *registry)
{
  registry->infos = NULL;
}

void voxy_name_registry_fini(struct voxy_name_registry *registry)
{
  arrfree(registry->infos);
}

voxy_name_id_t voxy_name_registry_register_name(struct voxy_name_registry *registry, struct voxy_name_info name_info)
{
  const voxy_name_id_t id = arrlenu(registry->infos);
  arrput(registry->infos, name_info);
  return id;
}

struct voxy_name_info voxy_name_registry_query_name(struct voxy_name_registry *registry, voxy_name_id_t id)
{
  assert(id < arrlenu(registry->infos));
  return registry->infos[id];
}

