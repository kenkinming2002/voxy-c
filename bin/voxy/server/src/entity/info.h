#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>

struct entity;
struct chunk_manager;

struct entity_info
{
  const char *mod;
  const char *name;

  void(*update)(const struct entity *entity, struct chunk_manager *chunk_manager);
};

DYNAMIC_ARRAY_DEFINE(entity_infos, struct entity_info);

#endif // ENTITY_INFO_H
