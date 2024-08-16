#ifndef ITEM_PLANK_PLANK_H
#define ITEM_PLANK_PLANK_H

#include <voxy/scene/main_game/types/registry.h>

void plank_item_register(void);
item_id_t plank_item_id_get(void);

bool plank_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_PLANK_PLANK_H
