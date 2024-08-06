#ifndef BLOCK_ORE_IRON_ORE_IRON_H
#define BLOCK_ORE_IRON_ORE_IRON_H

#include <voxy/scene/main_game/types/registry.h>

void ore_iron_block_register(void);
block_id_t ore_iron_block_id_get(void);

void ore_iron_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void ore_iron_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_ORE_IRON_ORE_IRON_H
