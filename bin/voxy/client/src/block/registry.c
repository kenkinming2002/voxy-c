#include "registry.h"

#include <assert.h>

void block_registry_init(struct block_registry *registry)
{
  DYNAMIC_ARRAY_INIT(registry->infos);
}

void block_registry_fini(struct block_registry *registry)
{
  DYNAMIC_ARRAY_CLEAR(registry->infos);
}

block_id_t block_registry_register_block(struct block_registry *registry, struct block_info block_info)
{
  const block_id_t id = registry->infos.item_count;
  DYNAMIC_ARRAY_APPEND(registry->infos, block_info);
  return id;
}

struct block_info block_registry_query_block(struct block_registry *registry, block_id_t id)
{
  assert(id < registry->infos.item_count);
  return registry->infos.items[id];
}

