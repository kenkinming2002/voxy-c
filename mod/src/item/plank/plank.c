#include "plank.h"
#include "mod.h"

#include "item/item.h"
#include "block/plank/plank.h"

static item_id_t plank_item_id;

void plank_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "plank";
  item_info.texture =  "mod/assets/textures/plank_item.png";
  item_info.on_use = plank_item_on_use;

  plank_item_id = register_item_info(item_info);
}

item_id_t plank_item_id_get(void)
{
  return plank_item_id;
}

bool plank_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, plank_block_id_get());
}

