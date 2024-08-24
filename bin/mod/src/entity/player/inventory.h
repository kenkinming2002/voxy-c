#ifndef ENTITY_PLAYER_INVENTORY_H
#define ENTITY_PLAYER_INVENTORY_H

#include "player.h"
#include "ui/layout.h"

void player_entity_update_inventory(struct entity *entity, float dt, struct player_ui_layout ui_layout);

#endif // ENTITY_PLAYER_INVENTORY_H
