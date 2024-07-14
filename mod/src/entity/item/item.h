#ifndef ENTITY_ITEM_ITEM_H
#define ENTITY_ITEM_ITEM_H

#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

struct item_opaque
{
  struct item item;
};

entity_id_t item_entity_id(void);

void item_entity_init(struct entity *entity, struct item item);
void item_entity_fini(struct entity *entity);

void item_entity_update(struct entity *entity, float dt);

#endif // ENTITY_ITEM_ITEM_H
