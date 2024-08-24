#ifndef BLOCK_PLANK_PLANK_H
#define BLOCK_PLANK_PLANK_H

#include <voxy/scene/main_game/types/registry.h>

void plank_block_register(void);
block_id_t plank_block_id_get(void);

void plank_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void plank_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_PLANK_PLANK_H
