#ifndef VOXY_MAIN_GAME_ENTITY_ITEM_H
#define VOXY_MAIN_GAME_ENTITY_ITEM_H

#include <voxy/scene/main_game/types/entity.h>
#include <stdbool.h>

void item_entity_init(struct entity *entity, struct item item);
void item_entity_fini(struct entity *entity);

/// Test if an entity is a item entity, and return the item stored internally if
/// that were to be the case.
bool try_item_entity_get_item(struct entity *entity, struct item *item);

#endif // VOXY_MAIN_GAME_ENTITY_ITEM_H

