#ifndef REGISTRY_NAME_H
#define REGISTRY_NAME_H

#include <voxy/server/registry/name.h>
#include <libcommon/utils/dynamic_array.h>
#include <stdint.h>

struct voxy_name_registry
{
  DYNAMIC_ARRAY_DEFINE(,struct voxy_name_info) infos;
};

void voxy_name_registry_init(struct voxy_name_registry *registry);
void voxy_name_registry_fini(struct voxy_name_registry *registry);

#endif // NAME_REGISTRY_H
