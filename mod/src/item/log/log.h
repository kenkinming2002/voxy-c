#ifndef ITEM_LOG_LOG_H
#define ITEM_LOG_LOG_H

#include <voxy/scene/main_game/types/registry.h>

void log_item_register(void);
item_id_t log_item_id_get(void);

bool log_item_on_use(struct entity *entity, struct item *item);

#endif // ITEM_LOG_LOG_H
