#ifndef ITEM_STONE_STONE_H
#define ITEM_STONE_STONE_H

#include <voxy/scene/main_game/types/registry.h>

void stone_item_register(void);
item_id_t stone_item_id_get(void);

void stone_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_STONE_STONE_H
