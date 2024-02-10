#ifndef TYPES_MOD_H
#define TYPES_MOD_H

#include <stddef.h>
#include <voxy/math/vector.h>
#include <voxy/config.h>

/*
 * Represent a mod which is a shared library on disk. All members are symbols
 * from shared library.
 */
struct mod
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
};

int mod_load(struct mod *mod, const char *filepath);
void mod_unload(struct mod *mod);

#endif // TYPES_MOD_H
