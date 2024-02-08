#ifndef VOXY_RESOURCE_PACK_H
#define VOXY_RESOURCE_PACK_H

#include "gl.h"
#include "font_set.h"

#include <voxy/resource_pack.h>

struct resource_pack
{
  void *handle;

  const struct block_info         *block_infos;
  const struct block_texture_info *block_texture_infos;
  const struct item_info          *item_infos;

  size_t block_info_count;
  size_t block_texture_info_count;
  size_t item_info_count;

  void (*generate_blocks)(seed_t seed, ivec3_t position, uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
  fvec3_t (*generate_spawn)(seed_t seed);

  struct font_set             font_set;
  struct gl_array_texture_2d  block_array_texture;
  struct gl_texture_2d       *item_textures;
};

int resource_pack_load(struct resource_pack *resource_pack, const char *filepath);
void resource_pack_unload(struct resource_pack *resource_pack);


#endif // VOXY_RESOURCE_PACK_H
