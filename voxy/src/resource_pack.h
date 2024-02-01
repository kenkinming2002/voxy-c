#ifndef VOXY_RESOURCE_PACK_H
#define VOXY_RESOURCE_PACK_H

#include <voxy/resource_pack.h>

struct resource_pack
{
  void *handle;

  const struct block_info         *block_infos;
  const struct block_texture_info *block_texture_infos;

  size_t block_info_count;
  size_t block_texture_info_count;

  void (*generate_heights)(seed_t seed, ivec2_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH]);
  void (*generate_tiles)(seed_t seed, ivec3_t position, float heights[CHUNK_WIDTH][CHUNK_WIDTH], uint8_t tiles[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
};

int resource_pack_load(struct resource_pack *resource_pack, const char *filepath);
void resource_pack_unload(struct resource_pack *resource_pack);


#endif // VOXY_RESOURCE_PACK_H
