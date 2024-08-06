#ifndef ITEM_ORE_COPPER_ORE_COPPER_H
#define ITEM_ORE_COPPER_ORE_COPPER_H

#include <voxy/scene/main_game/types/registry.h>

void ore_copper_item_register(void);
item_id_t ore_copper_item_id_get(void);

void ore_copper_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_ORE_COPPER_ORE_COPPER_H
