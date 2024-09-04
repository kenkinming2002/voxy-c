#ifndef BLOCK_REGISTRY_H
#define BLOCK_REGISTRY_H

#include <voxy/client/block/registry.h>

struct voxy_block_registry
{
  struct voxy_block_infos infos;
};

void voxy_block_registry_init(struct voxy_block_registry *registry);
void voxy_block_registry_fini(struct voxy_block_registry *registry);

#endif // BLOCK_REGISTRY_H
