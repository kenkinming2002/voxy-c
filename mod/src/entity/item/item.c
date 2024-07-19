#include "item.h"

#include <stdlib.h>

entity_id_t item_entity_id(void)
{
  static entity_id_t id = ENTITY_NONE;
  if(id == ENTITY_NONE)
  {
    struct entity_info entity_info;

    entity_info.mod = "main";
    entity_info.name = "item";

    entity_info.mesh = NULL;
    entity_info.texture = NULL;

    entity_info.hitbox_dimension = fvec3(0.1f, 0.1f, 0.1f);
    entity_info.hitbox_offset = fvec3_zero();
    entity_info.on_update = item_entity_update;

    id = register_entity_info(entity_info);
  }
  return id;
}

void item_entity_init(struct entity *entity, struct item item)
{
  struct item_opaque *opaque = malloc(sizeof *opaque);
  opaque->item = item;

  entity->id = item_entity_id();
  entity->opaque = opaque;
}

void item_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

void item_entity_update(struct entity *entity, float dt)
{
  (void)entity;
  (void)dt;
}

