#ifndef ENTITY_REGISTRY_H
#define ENTITY_REGISTRY_H

#include <voxy/server/entity/registry.h>

struct voxy_entity_registry
{
  struct voxy_entity_infos infos;
};

void voxy_entity_registry_init(struct voxy_entity_registry *registry);
void voxy_entity_registry_fini(struct voxy_entity_registry *registry);

#endif // ENTITY_REGISTRY_H
