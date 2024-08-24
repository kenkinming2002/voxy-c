#ifndef BLOCK_STONE_STONE_H
#define BLOCK_STONE_STONE_H

#include <voxy/scene/main_game/types/registry.h>

void stone_block_register(void);
block_id_t stone_block_id_get(void);

void stone_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void stone_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_STONE_STONE_H
