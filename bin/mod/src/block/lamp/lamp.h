#ifndef BLOCK_LAMP_LAMP_H
#define BLOCK_LAMP_LAMP_H

#include <voxy/scene/main_game/types/registry.h>

void lamp_block_register(void);
block_id_t lamp_block_id_get(void);

void lamp_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void lamp_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_LAMP_LAMP_H
