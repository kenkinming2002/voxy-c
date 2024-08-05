#ifndef BLOCK_WATER_WATER_H
#define BLOCK_WATER_WATER_H

#include <voxy/scene/main_game/types/registry.h>

void water_block_register(void);
block_id_t water_block_id_get(void);

void water_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void water_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_WATER_WATER_H
