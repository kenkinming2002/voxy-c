#include "grass.h"
#include "mod.h"

#include "item/item.h"
#include "block/grass/grass.h"

static item_id_t grass_item_id;

void grass_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "grass";
  item_info.texture =  "bin/mod/assets/textures/grass_item.png";
  item_info.on_use = grass_item_on_use;

  grass_item_id = register_item_info(item_info);
}

item_id_t grass_item_id_get(void)
{
  return grass_item_id;
}

bool grass_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, grass_block_id_get());
}

