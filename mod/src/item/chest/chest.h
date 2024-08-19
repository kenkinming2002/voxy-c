#ifndef ITEM_CHEST_CHEST_H
#define ITEM_CHEST_CHEST_H

#include <voxy/scene/main_game/types/registry.h>

void chest_item_register(void);
item_id_t chest_item_id_get(void);

bool chest_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_CHEST_CHEST_H
