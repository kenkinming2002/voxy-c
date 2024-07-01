#ifndef VOXY_MAIN_GAME_STATES_INVALIDATE_H
#define VOXY_MAIN_GAME_STATES_INVALIDATE_H

#include <voxy/main_game/types/chunk.h>

extern struct chunk *chunks_invalidated_light_head;
extern struct chunk *chunks_invalidated_light_tail;

extern struct chunk *chunks_invalidated_mesh_head;
extern struct chunk *chunks_invalidated_mesh_tail;

void world_invalidate_chunk_light(struct chunk *chunk);
void world_invalidate_chunk_mesh(struct chunk *chunk);

#endif // VOXY_MAIN_GAME_STATES_INVALIDATE_H
