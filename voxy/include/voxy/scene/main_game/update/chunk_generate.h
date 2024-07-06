#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H

#include <voxy/math/vector.h>

void enqueue_chunk_generate(ivec3_t position);
void update_chunk_generate(void);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_GENERATE_H
