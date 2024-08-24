#include "dynamite.h"
#include "mod.h"

#include "entity/dynamite/dynamite.h"

#include "item/item.h"

#include <voxy/scene/main_game/states/chunks.h>

#include <libcommon/utils/utils.h>

static item_id_t dynamite_item_id;

void dynamite_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "dynamite";
  item_info.texture = "bin/mod/assets/textures/dynamite_item.png";
  item_info.on_use = dynamite_item_on_use;

  dynamite_item_id = register_item_info(item_info);
}

item_id_t dynamite_item_id_get(void)
{
  return dynamite_item_id;
}

bool dynamite_item_on_use(struct entity *entity, struct item *item)
{
  const fvec3_t velocity = fvec3_mul_scalar(transform_forward(entity_transform(entity)), 10.0f);

  struct entity dynamite_entity;
  entity_init(&dynamite_entity, entity->position, entity->rotation, velocity, INFINITY, INFINITY);
  dynamite_entity_init(&dynamite_entity, 10.0f);
  if(!world_add_entity(dynamite_entity))
  {
    dynamite_entity_fini(&dynamite_entity);
    return false;
  }

  item->count -= 1;
  if(item->count == 0)
    item->id = ITEM_NONE;

  return true;
}

