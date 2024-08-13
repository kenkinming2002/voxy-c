#ifndef ITEM_MYSTERIOUS_FOOD_MYSTERIOUS_FOOD_H
#define ITEM_MYSTERIOUS_FOOD_MYSTERIOUS_FOOD_H

#include <voxy/scene/main_game/types/registry.h>

void mysterious_food_item_register(void);
item_id_t mysterious_food_item_id_get(void);

bool mysterious_food_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_MYSTERIOUS_FOOD_MYSTERIOUS_FOOD_H
