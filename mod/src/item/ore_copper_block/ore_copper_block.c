#include "ore_copper_block.h"
#include "mod.h"

#include "item/item.h"
#include "block/ore_copper/ore_copper.h"

static item_id_t ore_copper_block_item_id;

void ore_copper_block_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "ore_copper_block";
  item_info.texture =  "mod/assets/textures/ore_copper_block_item.png";
  item_info.on_use = ore_copper_block_item_on_use;

  ore_copper_block_item_id = register_item_info(item_info);
}

item_id_t ore_copper_block_item_id_get(void)
{
  return ore_copper_block_item_id;
}

void ore_copper_block_item_on_use(struct entity *entity, struct item *item)
{
  item_on_use_place_block(entity, item, ore_copper_block_id_get());
}

