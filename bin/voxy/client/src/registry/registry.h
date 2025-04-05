#ifndef REGISTRY_NAME_H
#define REGISTRY_NAME_H

#include <voxy/client/registry/name.h>

struct voxy_name_registry
{
  struct voxy_name_info *infos;
};

void voxy_name_registry_init(struct voxy_name_registry *registry);
void voxy_name_registry_fini(struct voxy_name_registry *registry);

#endif // NAME_REGISTRY_H
