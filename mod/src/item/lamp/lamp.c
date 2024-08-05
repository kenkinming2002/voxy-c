#include "lamp.h"
#include "mod.h"

#include "item/item.h"
#include "block/lamp/lamp.h"

static item_id_t lamp_item_id;

void lamp_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "lamp";
  item_info.texture =  "assets/textures/leave_item.png";
  item_info.on_use = lamp_item_on_use;

  lamp_item_id = register_item_info(item_info);
}

item_id_t lamp_item_id_get(void)
{
  return lamp_item_id;
}

void lamp_item_on_use(struct entity *entity, struct item *item)
{
  item_on_use_place_block(entity, item, lamp_block_id_get());
}

