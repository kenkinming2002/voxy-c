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

int item_entity_serialize(const struct entity *entity, struct serializer *serializer);
int item_entity_deserialize(struct entity *entity, struct deserializer *deserializer);

void item_entity_update(struct entity *entity, float dt);
void item_entity_render(const struct entity *entity, const struct camera *camera);

#endif // ENTITY_ITEM_ITEM_H
