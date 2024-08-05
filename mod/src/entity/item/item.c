#include "mod.h"
#include "item.h"

#include <stdlib.h>

static entity_id_t item_entity_id = ENTITY_NONE;

void item_entity_register(void)
{
  struct entity_info entity_info;

  entity_info.mod = MOD;
  entity_info.name = "item";

  entity_info.mesh = NULL;
  entity_info.texture = NULL;

  entity_info.hitbox_dimension = fvec3(0.1f, 0.1f, 0.1f);
  entity_info.hitbox_offset = fvec3_zero();

  entity_info.on_dispose = NULL;

  entity_info.on_save = item_entity_save;
  entity_info.on_load = item_entity_load;

  entity_info.on_update = item_entity_update;

  item_entity_id = register_entity_info(entity_info);
}

entity_id_t item_entity_id_get(void)
{
  return item_entity_id;
}

void item_entity_init(struct entity *entity, struct item item)
{
  struct item_opaque *opaque = malloc(sizeof *opaque);
  opaque->item = item;

  entity->id = item_entity_id;
  entity->opaque = opaque;
}

void item_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

bool item_entity_save(const struct entity *entity, FILE *file)
{
  const struct item_opaque *opaque = entity->opaque;
  return fwrite(opaque, sizeof *opaque, 1, file) == 1;
}

bool item_entity_load(struct entity *entity, FILE *file)
{
  struct item_opaque *opaque = malloc(sizeof *opaque);
  entity->opaque = opaque;
  return fread(opaque, sizeof *opaque, 1, file) == 1;
}

void item_entity_update(struct entity *entity, float dt)
{
  (void)entity;
  (void)dt;
}

