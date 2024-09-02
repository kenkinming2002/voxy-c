#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>
#include <libcommon/graphics/camera.h>

struct entity;
struct entity_info
{
  const char *mod;
  const char *name;

  void(*render)(const struct entity *entity, const struct camera *camera);
};

DYNAMIC_ARRAY_DEFINE(entity_infos, struct entity_info);

#endif // ENTITY_INFO_H
