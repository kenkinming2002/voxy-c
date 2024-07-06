#include <voxy/scene/main_game/entity/item.h>
#include <voxy/scene/main_game/types/item.h>

#include <stdlib.h>
#include <assert.h>

struct item_opaque
{
  struct item item;
};

static void item_entity_update(struct entity *entity, float dt);
static entity_id_t item_entity_id()
{
  static entity_id_t id = ENTITY_NONE;
  if(id == ENTITY_NONE)
  {
    struct entity_info entity_info;
    entity_info.mod = "main";
    entity_info.name = "item";
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

static void item_entity_update(struct entity *entity, float dt)
{
  (void)entity;
  (void)dt;
}

bool try_item_entity_get_item(struct entity *entity, struct item *item)
{
  if(entity->id != item_entity_id())
    return false;

  struct item_opaque *opaque = entity->opaque;
  *item = opaque->item;
  return true;
}

bool is_item_entity(struct entity *entity)
{
  return entity->id == item_entity_id();
}

struct item item_entity_get_item(struct entity *entity)
{
  assert(is_item_entity(entity));
  struct item_opaque *opaque = entity->opaque;
  return opaque->item;
}

void item_entity_set_item(struct entity *entity, struct item item)
{
  assert(is_item_entity(entity));
  struct item_opaque *opaque = entity->opaque;
  opaque->item = item;
}

bool item_entity_should_destroy(struct entity *entity)
{
  assert(is_item_entity(entity));
  struct item_opaque *opaque = entity->opaque;
  return opaque->item.id == ITEM_NONE;
}
