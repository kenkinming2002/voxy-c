#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include <libcommon/math/direction.h>
#include <libcommon/utils/dynamic_array.h>

enum block_type
{
  BLOCK_TYPE_INVISIBLE,
  BLOCK_TYPE_TRANSPARENT,
  BLOCK_TYPE_OPAQUE,
};

struct block_info
{
  const char *mod;
  const char *name;

  enum block_type type;
  const char *textures[DIRECTION_COUNT];
};

DYNAMIC_ARRAY_DEFINE(block_infos, struct block_info);

#endif // BLOCK_INFO_H
