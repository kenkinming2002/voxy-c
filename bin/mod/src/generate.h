#ifndef GENERATE_H
#define GENERATE_H

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/config.h>

#include <libcommon/math/vector.h>
#include <libcommon/math/random.h>

void base_generate_chunk_blocks(seed_t seed, ivec3_t position, block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
fvec3_t base_generate_player_spawn(seed_t seed);

#endif // GENERATE_H
