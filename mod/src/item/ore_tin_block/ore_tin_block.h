#ifndef ITEM_ORE_TIN_BLOCK_ORE_TIN_BLOCK_H
#define ITEM_ORE_TIN_BLOCK_ORE_TIN_BLOCK_H

#include <voxy/scene/main_game/types/registry.h>

void ore_tin_block_item_register(void);
item_id_t ore_tin_block_item_id_get(void);

void ore_tin_block_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_ORE_TIN_BLOCK_ORE_TIN_BLOCK_H
