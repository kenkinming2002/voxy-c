#ifndef VOXY_SERVER_BLOCK_INFO_H
#define VOXY_SERVER_BLOCK_INFO_H

#include <libcommon/math/direction.h>
#include <libcommon/utils/dynamic_array.h>

struct voxy_block_info
{
  const char *mod;
  const char *name;
  bool collide;
  uint8_t light_level;
};

DYNAMIC_ARRAY_DEFINE(voxy_block_infos, struct voxy_block_info);

#endif // VOXY_SERVER_BLOCK_INFO_H
