#ifndef ITEM_ORE_COAL_ORE_COAL_H
#define ITEM_ORE_COAL_ORE_COAL_H

#include <voxy/scene/main_game/types/registry.h>

void ore_coal_block_item_register(void);
item_id_t ore_coal_block_item_id_get(void);

void ore_coal_block_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_ORE_COAL_ORE_COAL_H
