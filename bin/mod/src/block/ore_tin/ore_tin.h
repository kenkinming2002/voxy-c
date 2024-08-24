#ifndef BLOCK_ORE_TIN_ORE_TIN_H
#define BLOCK_ORE_TIN_ORE_TIN_H

#include <voxy/scene/main_game/types/registry.h>

void ore_tin_block_register(void);
block_id_t ore_tin_block_id_get(void);

void ore_tin_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void ore_tin_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_ORE_TIN_ORE_TIN_H
