#ifndef ENTITY_ITEM_ITEM_H
#define ENTITY_ITEM_ITEM_H

#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

struct item_opaque
{
  struct item item;
};

void item_entity_register(void);
entity_id_t item_entity_id_get(void);

void item_entity_init(struct entity *entity, struct item item);
void item_entity_fini(struct entity *entity);

bool item_entity_save(const struct entity *entity, FILE *file);
bool item_entity_load(struct entity *entity, FILE *file);

void item_entity_update(struct entity *entity, float dt);

#endif // ENTITY_ITEM_ITEM_H
