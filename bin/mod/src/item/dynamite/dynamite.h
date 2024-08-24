#ifndef ITEM_DYNAMITE_DYNAMITE_H
#define ITEM_DYNAMITE_DYNAMITE_H

#include <voxy/scene/main_game/types/registry.h>

void dynamite_item_register(void);
item_id_t dynamite_item_id_get(void);

bool dynamite_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_DYNAMITE_DYNAMITE_H
