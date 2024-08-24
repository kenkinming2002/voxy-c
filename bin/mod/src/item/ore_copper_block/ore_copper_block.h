#ifndef ITEM_ORE_COPPER_BLOCK_ORE_COPPER_BLOCK_H
#define ITEM_ORE_COPPER_BLOCK_ORE_COPPER_BLOCK_H

#include <voxy/scene/main_game/types/registry.h>

void ore_copper_block_item_register(void);
item_id_t ore_copper_block_item_id_get(void);

bool ore_copper_block_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_ORE_COPPER_BLOCK_ORE_COPPER_BLOCK_H
