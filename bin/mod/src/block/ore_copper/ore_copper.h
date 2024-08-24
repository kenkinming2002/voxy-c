#ifndef BLOCK_ORE_COPPER_ORE_COPPER_H
#define BLOCK_ORE_COPPER_ORE_COPPER_H

#include <voxy/scene/main_game/types/registry.h>

void ore_copper_block_register(void);
block_id_t ore_copper_block_id_get(void);

void ore_copper_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void ore_copper_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_ORE_COPPER_ORE_COPPER_H
