#include "leave.h"
#include "mod.h"

#include "item/item.h"
#include "block/leave/leave.h"

static item_id_t leave_item_id;

void leave_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "leave";
  item_info.texture =  "mod/assets/textures/leave_item.png";
  item_info.on_use = leave_item_on_use;

  leave_item_id = register_item_info(item_info);
}

item_id_t leave_item_id_get(void)
{
  return leave_item_id;
}

bool leave_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, leave_block_id_get());
}

