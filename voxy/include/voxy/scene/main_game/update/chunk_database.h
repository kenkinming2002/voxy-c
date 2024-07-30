#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H

#include <stdbool.h>
#include <voxy/math/vector.h>
#include <voxy/scene/main_game/types/chunk_data.h>

struct chunk_data *load_chunk_data(ivec3_t position);
bool save_chunk_data(ivec3_t position, struct chunk_data *chunk_data);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H
