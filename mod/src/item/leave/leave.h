#ifndef ITEM_LEAVE_LEAVE_H
#define ITEM_LEAVE_LEAVE_H

#include <voxy/scene/main_game/types/registry.h>

void leave_item_register(void);
item_id_t leave_item_id_get(void);

bool leave_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_LEAVE_LEAVE_H
