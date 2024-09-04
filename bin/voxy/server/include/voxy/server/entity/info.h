#ifndef VOXY_SERVER_ENTITY_INFO_H
#define VOXY_SERVER_ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>

struct voxy_entity;
struct voxy_context;

struct voxy_entity_info
{
  const char *mod;
  const char *name;

  void(*update)(const struct voxy_entity *entity, const struct voxy_context *context);
};

DYNAMIC_ARRAY_DEFINE(voxy_entity_infos, struct voxy_entity_info);

#endif // VOXY_SERVER_ENTITY_INFO_H
