#ifndef BLOCK_LEAVE_LEAVE_H
#define BLOCK_LEAVE_LEAVE_H

#include <voxy/scene/main_game/types/registry.h>

void leave_block_register(void);
block_id_t leave_block_id_get(void);

void leave_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void leave_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_LEAVE_LEAVE_H
