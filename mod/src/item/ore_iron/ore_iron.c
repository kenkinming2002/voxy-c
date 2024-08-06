#include "ore_iron.h"
#include "mod.h"

#include "item/item.h"
#include "block/ore_iron/ore_iron.h"

static item_id_t ore_iron_item_id;

void ore_iron_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "ore_iron";
  item_info.texture =  "mod/assets/textures/ore_iron_item.png";
  item_info.on_use = ore_iron_item_on_use;

  ore_iron_item_id = register_item_info(item_info);
}

item_id_t ore_iron_item_id_get(void)
{
  return ore_iron_item_id;
}

void ore_iron_item_on_use(struct entity *entity, struct item *item)
{
  item_on_use_place_block(entity, item, ore_iron_block_id_get());
}

