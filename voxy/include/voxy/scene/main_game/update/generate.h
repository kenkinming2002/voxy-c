#ifndef VOXY_MAIN_GAME_UPDATE_GENERATE_H
#define VOXY_MAIN_GAME_UPDATE_GENERATE_H

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/types/registry.h>

#include <voxy/math/vector.h>

// FIXME: Come up with a better interface that allows possibly multiple mods to
//        control the procedural generation such as concept of biomes,
//        structures, etc.

typedef void(*generate_chunk_blocks_t)(seed_t seed, ivec3_t position, block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
typedef fvec3_t(*generate_player_spawn_t)(seed_t seed);

void register_generate_chunk_blocks(generate_chunk_blocks_t func);
void register_generate_player_spawn(generate_player_spawn_t func);

void generate_chunk_blocks(seed_t seed, ivec3_t position, block_id_t blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH]);
fvec3_t generate_player_spawn(seed_t seed);

#endif // VOXY_MAIN_GAME_UPDATE_GENERATE_H
