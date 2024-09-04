#include "registry.h"

#include <assert.h>

void voxy_block_registry_init(struct voxy_block_registry *registry)
{
  DYNAMIC_ARRAY_INIT(registry->infos);
}

void voxy_block_registry_fini(struct voxy_block_registry *registry)
{
  DYNAMIC_ARRAY_CLEAR(registry->infos);
}

block_id_t voxy_block_registry_register_block(struct voxy_block_registry *registry, struct voxy_block_info block_info)
{
  const block_id_t id = registry->infos.item_count;
  DYNAMIC_ARRAY_APPEND(registry->infos, block_info);
  return id;
}

struct voxy_block_info voxy_block_registry_query_block(struct voxy_block_registry *registry, block_id_t id)
{
  assert(id < registry->infos.item_count);
  return registry->infos.items[id];
}

