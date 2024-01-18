#ifndef VOXY_INTERFACE_RESOURCE_PACK_H
#define VOXY_INTERFACE_RESOURCE_PACK_H

#include <stddef.h>

struct block_info
{
  const char *name;

  size_t texture_left;
  size_t texture_right;
  size_t texture_back;
  size_t texture_front;
  size_t texture_bottom;
  size_t texture_top;
};

struct block_texture_info
{
  const char *filepath;
};

extern const struct block_info         block_infos[];
extern const struct block_texture_info block_texture_infos[];

extern const size_t block_info_count;
extern const size_t block_texture_info_count;

#endif // VOXY_INTERFACE_RESOURCE_PACK_H
