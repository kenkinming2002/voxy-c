#ifndef BLOCK_ETHER_ETHER_H
#define BLOCK_ETHER_ETHER_H

#include <voxy/scene/main_game/types/registry.h>

void ether_block_register(void);
block_id_t ether_block_id_get(void);

void ether_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void ether_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_ETHER_ETHER_H
