#include "log.h"
#include "mod.h"

#include "item/item.h"
#include "block/log/log.h"

static item_id_t log_item_id;

void log_item_register(void)
{
  struct item_info item_info;

  item_info.mod = MOD;
  item_info.name = "log";
  item_info.texture =  "mod/assets/textures/log_item.png";
  item_info.on_use = log_item_on_use;

  log_item_id = register_item_info(item_info);
}

item_id_t log_item_id_get(void)
{
  return log_item_id;
}

bool log_item_on_use(struct entity *entity, struct item *item)
{
  return item_on_use_place_block(entity, item, log_block_id_get());
}

