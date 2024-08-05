#ifndef BLOCK_GRASS_GRASS_H
#define BLOCK_GRASS_GRASS_H

#include <voxy/scene/main_game/types/registry.h>

void grass_block_register(void);
block_id_t grass_block_id_get(void);

void grass_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void grass_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_GRASS_GRASS_H
