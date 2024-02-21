#ifndef VOXY_MAIN_GAME_MOD_H
#define VOXY_MAIN_GAME_MOD_H

#include <voxy/main_game/mod_interface.h>
#include <voxy/math/random.h>
#include <voxy/math/vector.h>

#define MOD_FUNCTIONS \
  X(generate_blocks, void, seed_t seed, ivec3_t position, block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]) \
  X(generate_spawn, fvec3_t, seed_t seed)

#define X(name, ret, ...) ret mod_##name(__VA_ARGS__);
MOD_FUNCTIONS
#undef X

void mod_load(const char *filepath);

#endif // VOXY_MAIN_GAME_MOD_H
