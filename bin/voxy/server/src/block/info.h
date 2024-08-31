#ifndef BLOCK_INFO_H
#define BLOCK_INFO_H

#include <libcommon/math/direction.h>
#include <libcommon/utils/dynamic_array.h>

struct block_info
{
  const char *mod;
  const char *name;
};

DYNAMIC_ARRAY_DEFINE(block_infos, struct block_info);

#endif // BLOCK_INFO_H
