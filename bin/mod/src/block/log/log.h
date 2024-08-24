#ifndef BLOCK_LOG_LOG_H
#define BLOCK_LOG_LOG_H

#include <voxy/scene/main_game/types/registry.h>

void log_block_register(void);
block_id_t log_block_id_get(void);

void log_block_on_create(struct entity *entity, struct chunk *chunk, ivec3_t position);
void log_block_on_destroy(struct entity *entity, struct chunk *chunk, ivec3_t position);

#endif // BLOCK_LOG_LOG_H
