#ifndef BLOCK_BLOCK_H
#define BLOCK_BLOCK_H

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/math/vector.h>

void block_on_destroy_spawn_item(struct chunk *chunk, ivec3_t position, item_id_t item_id);

#endif // BLOCK_BLOCK_H
