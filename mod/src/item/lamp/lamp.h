#ifndef ITEM_LAMP_LAMP_H
#define ITEM_LAMP_LAMP_H

#include <voxy/scene/main_game/types/registry.h>

void lamp_item_register(void);
item_id_t lamp_item_id_get(void);

void lamp_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_LAMP_LAMP_H
