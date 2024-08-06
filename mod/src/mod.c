#include "generate.h"

#include "entity/item/item.h"
#include "entity/player/player.h"
#include "entity/weird/weird.h"

#include "block/empty/empty.h"
#include "block/ether/ether.h"
#include "block/grass/grass.h"
#include "block/lamp/lamp.h"
#include "block/leave/leave.h"
#include "block/log/log.h"
#include "block/ore_coal/ore_coal.h"
#include "block/ore_copper/ore_copper.h"
#include "block/ore_iron/ore_iron.h"
#include "block/ore_tin/ore_tin.h"
#include "block/stone/stone.h"
#include "block/water/water.h"

#include "item/grass/grass.h"
#include "item/lamp/lamp.h"
#include "item/leave/leave.h"
#include "item/log/log.h"
#include "item/ore_coal/ore_coal.h"
#include "item/ore_copper/ore_copper.h"
#include "item/ore_iron/ore_iron.h"
#include "item/ore_tin/ore_tin.h"
#include "item/stone/stone.h"

#include "update/spawn_player.h"
#include "update/spawn_weird.h"

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/update/generate.h>

#include <stdbool.h>

void mod_init()
{
  item_entity_register();
  player_entity_register();
  weird_entity_register();

  empty_block_register();
  ether_block_register();
  grass_block_register();
  lamp_block_register();
  leave_block_register();
  log_block_register();
  ore_coal_block_register();
  ore_copper_block_register();
  ore_iron_block_register();
  ore_tin_block_register();
  stone_block_register();
  water_block_register();

  grass_item_register();
  lamp_item_register();
  leave_item_register();
  log_item_register();
  ore_coal_item_register();
  ore_copper_item_register();
  ore_iron_item_register();
  ore_tin_item_register();
  stone_item_register();

  register_generate_chunk_blocks(&base_generate_chunk_blocks);
  register_generate_player_spawn(&base_generate_player_spawn);
}

void mod_enter(void)
{
  spawn_player_enter();
}

void mod_leave(void)
{
  spawn_player_leave();
}

void mod_update(void)
{
  spawn_player_update();
  spawn_weird_update();
}

