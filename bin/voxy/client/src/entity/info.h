#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>
#include <libcommon/graphics/camera.h>

struct voxy_entity;
struct voxy_entity_info
{
  const char *mod;
  const char *name;

  void(*render)(const struct voxy_entity *entity, const struct camera *camera);
};

DYNAMIC_ARRAY_DEFINE(entity_infos, struct voxy_entity_info);

#endif // ENTITY_INFO_H
