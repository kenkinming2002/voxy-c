#ifndef VOXY_SERVER_REGISTRY_BLOCK_INFO_H
#define VOXY_SERVER_REGISTRY_BLOCK_INFO_H

#include <libmath/direction.h>

struct voxy_block_info
{
  const char *mod;
  const char *name;
  bool collide;
  uint8_t light_level;
};

#endif // VOXY_SERVER_REGISTRY_BLOCK_INFO_H
