#ifndef ENTITY_REGISTRY_H
#define ENTITY_REGISTRY_H

#include <voxy/client/entity/registry.h>

#include <stdint.h>

typedef uint8_t entity_id_t;

struct voxy_entity_registry
{
  struct voxy_entity_infos infos;
};

void voxy_entity_registry_init(struct voxy_entity_registry *registry);
void voxy_entity_registry_fini(struct voxy_entity_registry *registry);

#endif // ENTITY_REGISTRY_H
