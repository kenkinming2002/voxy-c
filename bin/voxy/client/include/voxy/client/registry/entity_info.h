#ifndef VOXY_CLIENT_REGISTRY_ENTITY_INFO_H
#define VOXY_CLIENT_REGISTRY_ENTITY_INFO_H

#include <libcommon/graphics/camera.h>

struct voxy_entity;
struct voxy_entity_info
{
  const char *mod;
  const char *name;

  void(*render)(const struct voxy_entity *entity, const struct camera *camera);
};

#endif // VOXY_CLIENT_REGISTRY_ENTITY_INFO_H
