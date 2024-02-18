#ifndef VOXY_MOD_INTERFACE_H
#define VOXY_MOD_INTERFACE_H

#include <voxy/main_game/config.h>
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

struct item_info
{
  const char *name;
  const char *texture_filepath;
  void(*on_use)(uint8_t item_id);
};

extern const struct block_info         block_infos[];
extern const struct block_texture_info block_texture_infos[];
extern const struct item_info          item_infos[];

extern const size_t block_info_count;
extern const size_t block_texture_info_count;
extern const size_t item_info_count;

void generate_blocks(seed_t seed, ivec3_t position, uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
fvec3_t generate_spawn(seed_t seed);

#endif // VOXY_MOD_INTERFACE_H
