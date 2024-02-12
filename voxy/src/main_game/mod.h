#ifndef MAIN_GAME_MOD_H
#define MAIN_GAME_MOD_H

#include <voxy/mod_interface.h>
#include <voxy/math/random.h>
#include <voxy/math/vector.h>

#define MOD_ARRAYS \
  X(block_info) \
  X(block_texture_info) \
  X(item_info)

#define MOD_FUNCTIONS \
  X(generate_blocks, void, seed_t seed, ivec3_t position, uint8_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]) \
  X(generate_spawn, fvec3_t, seed_t seed)

#define X(name) const struct name *mod_##name##_get(uint8_t id);
MOD_ARRAYS
#undef X

#define X(name) uint8_t mod_##name##_count_get();
MOD_ARRAYS
#undef X

#define X(name, ret, ...) ret mod_##name(__VA_ARGS__);
MOD_FUNCTIONS
#undef X

void mod_load(const char *filepath);

#endif // MAIN_GAME_MOD_H
