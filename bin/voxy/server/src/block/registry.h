#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <voxy/server/block/registry.h>

typedef uint8_t block_id_t;

struct voxy_block_registry
{
  struct voxy_block_infos infos;
};

void voxy_block_registry_init(struct voxy_block_registry *registry);
void voxy_block_registry_fini(struct voxy_block_registry *registry);

#endif // BLOCK_REGISTRY_H
