#ifndef BLOCK_EMPTY_EMPTY_H
#define BLOCK_EMPTY_EMPTY_H

#include <voxy/scene/main_game/types/registry.h>

void empty_block_register(void);
block_id_t empty_block_id_get(void);

void empty_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void empty_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_EMPTY_EMPTY_H
