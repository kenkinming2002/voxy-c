#ifndef VOXY_INTERFACE_RESOURCE_PACK_H
#define VOXY_INTERFACE_RESOURCE_PACK_H

#include <voxy/config.h>
#include <voxy/math/noise.h>

#include <stddef.h>
#include <stdint.h>

enum block_type
{
  BLOCK_TYPE_INVISIBLE,
  BLOCK_TYPE_TRANSPARENT,
  BLOCK_TYPE_OPAQUE,
};

struct block_info
{
  const char *name;

  enum block_type type;

  uint8_t ether       : 1;
  uint8_t light_level : 4;

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

void generate_heights(seed_t seed, ivec2_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH]);
void generate_tiles(seed_t seed, ivec3_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH], uint8_t tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);

#endif // VOXY_INTERFACE_RESOURCE_PACK_H
