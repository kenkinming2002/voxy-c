#include "mysterious_food.h"
#include "mod.h"

#include "item/item.h"

#include <libcommon/utils/utils.h>

static item_id_t mysterious_food_item_id;

void mysterious_food_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "mysterious food";
  item_info.texture = "bin/mod/assets/textures/mysterious_food_item.png";
  item_info.on_use = mysterious_food_item_on_use;

  mysterious_food_item_id = register_item_info(item_info);
}

item_id_t mysterious_food_item_id_get(void)
{
  return mysterious_food_item_id;
}

bool mysterious_food_item_on_use(struct entity *entity, struct item *item)
{
  if(item->count > 0)
  {
    entity->health += 2.0f;
    entity->health = MIN(entity->health, entity->max_health);
    if(--item->count == 0)
      item->id = ITEM_NONE;

    return true;
  }

  return false;
}

