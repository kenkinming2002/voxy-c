#include "chest.h"
#include "mod.h"

#include "item/item.h"
#include "block/chest/chest.h"

static item_id_t chest_item_id;

void chest_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "chest";
  item_info.texture =  "mod/assets/textures/chest_item.png";
  item_info.on_use = chest_item_on_use;

  chest_item_id = register_item_info(item_info);
}

item_id_t chest_item_id_get(void)
{
  return chest_item_id;
}

bool chest_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, chest_block_id_get());
}

