#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H

#include <voxy/scene/main_game/types/chunk.h>

bool is_chunk_generating(ivec3_t position);
struct chunk *generate_chunk(ivec3_t position);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H
