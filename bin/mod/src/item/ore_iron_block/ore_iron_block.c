#include "ore_iron_block.h"
#include "mod.h"

#include "item/item.h"
#include "block/ore_iron/ore_iron.h"

static item_id_t ore_iron_block_item_id;

void ore_iron_block_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "iron ore block";
  item_info.texture =  "bin/mod/assets/textures/ore_iron_block_item.png";
  item_info.on_use = ore_iron_block_item_on_use;

  ore_iron_block_item_id = register_item_info(item_info);
}

item_id_t ore_iron_block_item_id_get(void)
{
  return ore_iron_block_item_id;
}

bool ore_iron_block_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, ore_iron_block_id_get());
}

