#ifndef ENTITY_INFO_H
#define ENTITY_INFO_H

#include <libcommon/utils/dynamic_array.h>

struct entity_info
{
  const char *mod;
  const char *name;
};

DYNAMIC_ARRAY_DEFINE(entity_infos, struct entity_info);

#endif // ENTITY_INFO_H
