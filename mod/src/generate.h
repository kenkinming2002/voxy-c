#ifndef MOD_GENERATE_H
#define MOD_GENERATE_H

#include <voxy/main_game/types/registry.h>
#include <voxy/main_game/config.h>

#include <voxy/math/vector.h>
#include <voxy/math/random.h>

void base_generate_chunk_blocks(seed_t seed, ivec3_t position, block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
fvec3_t base_generate_player_spawn(seed_t seed);

#endif // MOD_GENERATE_H
