#ifndef VOXY_MAIN_GAME_MOD_INTERFACE_H
#define VOXY_MAIN_GAME_MOD_INTERFACE_H

#include <voxy/main_game/config.h>
#include <voxy/main_game/registry.h>
#include <voxy/math/noise.h>

#include <stddef.h>
#include <stdint.h>

void generate_blocks(seed_t seed, ivec3_t position, block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
fvec3_t generate_spawn(seed_t seed);

#endif // VOXY_MAIN_GAME_MOD_INTERFACE_H
