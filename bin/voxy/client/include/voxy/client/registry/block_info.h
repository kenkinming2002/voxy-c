#ifndef VOXY_CLIENT_REGISTRY_BLOCK_INFO_H
#define VOXY_CLIENT_REGISTRY_BLOCK_INFO_H

#include <libmath/direction.h>

enum voxy_block_type
{
  VOXY_BLOCK_TYPE_INVISIBLE,
  VOXY_BLOCK_TYPE_TRANSPARENT,
  VOXY_BLOCK_TYPE_OPAQUE,
};

struct voxy_block_info
{
  const char *mod;
  const char *name;

  enum voxy_block_type type;
  const char *textures[DIRECTION_COUNT];
};

#endif // VOXY_CLIENT_REGISTRY_BLOCK_INFO_H
