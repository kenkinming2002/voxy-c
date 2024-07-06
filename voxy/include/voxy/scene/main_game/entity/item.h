#ifndef VOXY_SCENE_MAIN_GAME_ENTITY_ITEM_H
#define VOXY_SCENE_MAIN_GAME_ENTITY_ITEM_H

#include <voxy/scene/main_game/types/entity.h>
#include <stdbool.h>

void item_entity_init(struct entity *entity, struct item item);
void item_entity_fini(struct entity *entity);

bool is_item_entity(struct entity *entity);

struct item item_entity_get_item(struct entity *entity);
void item_entity_set_item(struct entity *entity, struct item item);

bool item_entity_should_destroy(struct entity *entity);

#endif // VOXY_SCENE_MAIN_GAME_ENTITY_ITEM_H
