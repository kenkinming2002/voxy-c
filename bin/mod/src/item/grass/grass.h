#ifndef ITEM_GRASS_GRASS_H
#define ITEM_GRASS_GRASS_H

#include <voxy/scene/main_game/types/registry.h>

void grass_item_register(void);
item_id_t grass_item_id_get(void);

bool grass_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_GRASS_GRASS_H
