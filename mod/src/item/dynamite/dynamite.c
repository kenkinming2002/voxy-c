#include "dynamite.h"
#include "mod.h"

#include "item/item.h"

#include <libcommon/utils/utils.h>

static item_id_t dynamite_item_id;

void dynamite_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "dynamite";
  item_info.texture = "mod/assets/textures/dynamite_item.png";
  item_info.on_use = dynamite_item_on_use;

  dynamite_item_id = register_item_info(item_info);
}

item_id_t dynamite_item_id_get(void)
{
  return dynamite_item_id;
}

bool dynamite_item_on_use(struct entity *entity, struct item *item)
{
  return false;
}

