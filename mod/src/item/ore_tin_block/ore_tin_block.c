#include "ore_tin_block.h"
#include "mod.h"

#include "item/item.h"
#include "block/ore_tin/ore_tin.h"

static item_id_t ore_tin_block_item_id;

void ore_tin_block_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "tin ore block";
  item_info.texture =  "mod/assets/textures/ore_tin_block_item.png";
  item_info.on_use = ore_tin_block_item_on_use;

  ore_tin_block_item_id = register_item_info(item_info);
}

item_id_t ore_tin_block_item_id_get(void)
{
  return ore_tin_block_item_id;
}

void ore_tin_block_item_on_use(struct entity *entity, struct item *item)
{
  item_on_use_place_block(entity, item, ore_tin_block_id_get());
}

