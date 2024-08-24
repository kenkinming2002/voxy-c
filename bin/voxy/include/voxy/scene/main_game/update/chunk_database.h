#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H

#include <voxy/scene/main_game/types/chunk.h>

#include <libcommon/math/vector.h>

#include <stdbool.h>

struct chunk *load_chunk(ivec3_t position);
bool save_chunk(struct chunk *chunk);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_DATABASE_H
