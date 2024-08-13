#include "stone.h"
#include "mod.h"

#include "item/item.h"
#include "block/stone/stone.h"

static item_id_t stone_item_id;

void stone_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "stone";
  item_info.texture =  "mod/assets/textures/stone_item.png";
  item_info.on_use = stone_item_on_use;

  stone_item_id = register_item_info(item_info);
}

item_id_t stone_item_id_get(void)
{
  return stone_item_id;
}

bool stone_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, stone_block_id_get());
}

