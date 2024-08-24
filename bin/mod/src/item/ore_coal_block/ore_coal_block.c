#include "ore_coal_block.h"
#include "mod.h"

#include "item/item.h"
#include "block/ore_coal/ore_coal.h"

static item_id_t ore_coal_block_item_id;

void ore_coal_block_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "coal ore block";
  item_info.texture =  "bin/mod/assets/textures/ore_coal_block_item.png";
  item_info.on_use = ore_coal_block_item_on_use;

  ore_coal_block_item_id = register_item_info(item_info);
}

item_id_t ore_coal_block_item_id_get(void)
{
  return ore_coal_block_item_id;
}

bool ore_coal_block_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, ore_coal_block_id_get());
}

