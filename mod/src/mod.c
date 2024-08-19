#include "generate.h"

#include "entity/item/item.h"
#include "entity/dynamite/dynamite.h"
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
#include "block/plank/plank.h"
#include "block/chest/chest.h"

#include "item/grass/grass.h"
#include "item/lamp/lamp.h"
#include "item/leave/leave.h"
#include "item/log/log.h"
#include "item/ore_coal_block/ore_coal_block.h"
#include "item/ore_copper_block/ore_copper_block.h"
#include "item/ore_iron_block/ore_iron_block.h"
#include "item/ore_tin_block/ore_tin_block.h"
#include "item/stone/stone.h"
#include "item/mysterious_food/mysterious_food.h"
#include "item/dynamite/dynamite.h"
#include "item/plank/plank.h"
#include "item/chest/chest.h"

#include "update/spawn_player.h"
#include "update/spawn_weird.h"

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/crafting/crafting.h>

#include <stdbool.h>

void mod_init()
{
  item_entity_register();
  player_entity_register();
  weird_entity_register();
  dynamite_entity_register();

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
  plank_block_register();
  chest_block_register();

  grass_item_register();
  lamp_item_register();
  leave_item_register();
  log_item_register();
  ore_coal_block_item_register();
  ore_copper_block_item_register();
  ore_iron_block_item_register();
  ore_tin_block_item_register();
  stone_item_register();
  mysterious_food_item_register();
  dynamite_item_register();
  plank_item_register();
  chest_item_register();

  register_generate_chunk_blocks(&base_generate_chunk_blocks);
  register_generate_player_spawn(&base_generate_player_spawn);

  struct recipe recipe;

  // Lamp
  {
    for(int j=0; j<3; ++j)
      for(int i=0; i<3; ++i)
      {
        recipe.inputs[j][i].id = ITEM_NONE;
        recipe.inputs[j][i].count = 0;
      }

    recipe.inputs[0][0].id = ore_coal_block_item_id_get();
    recipe.inputs[0][0].count = 1;

    recipe.output.id = lamp_item_id_get();
    recipe.output.count = 4;

    crafting_add_recipe(recipe);
  }

  // Mysterious food
  {
    for(int j=0; j<3; ++j)
      for(int i=0; i<3; ++i)
      {
        recipe.inputs[j][i].id = ITEM_NONE;
        recipe.inputs[j][i].count = 0;
      }

    recipe.inputs[0][0].id = ore_iron_block_item_id_get();
    recipe.inputs[0][0].count = 1;

    recipe.inputs[0][1].id = ore_copper_block_item_id_get();
    recipe.inputs[0][1].count = 1;

    recipe.inputs[0][2].id = ore_tin_block_item_id_get();
    recipe.inputs[0][2].count = 1;

    recipe.output.id = mysterious_food_item_id_get();
    recipe.output.count = 3;

    crafting_add_recipe(recipe);
  }

  // Dynamite
  {
    for(int j=0; j<3; ++j)
      for(int i=0; i<3; ++i)
      {
        recipe.inputs[j][i].id = ITEM_NONE;
        recipe.inputs[j][i].count = 0;
      }

    recipe.inputs[0][0].id = ore_coal_block_item_id_get();
    recipe.inputs[0][0].count = 1;

    recipe.inputs[0][1].id = ore_iron_block_item_id_get();
    recipe.inputs[0][1].count = 1;

    recipe.output.id = dynamite_item_id_get();
    recipe.output.count = 1;

    crafting_add_recipe(recipe);
  }

  // Plank
  {
    for(int j=0; j<3; ++j)
      for(int i=0; i<3; ++i)
      {
        recipe.inputs[j][i].id = ITEM_NONE;
        recipe.inputs[j][i].count = 0;
      }

    recipe.inputs[0][0].id = log_item_id_get();
    recipe.inputs[0][0].count = 1;

    recipe.output.id = plank_item_id_get();
    recipe.output.count = 4;

    crafting_add_recipe(recipe);
  }

  // Chest
  {
    for(int j=0; j<3; ++j)
      for(int i=0; i<3; ++i)
      {
        recipe.inputs[j][i].id = plank_item_id_get();
        recipe.inputs[j][i].count = 1;
      }

    recipe.inputs[1][1].id = ITEM_NONE;
    recipe.inputs[1][1].count = 0;

    recipe.output.id = chest_item_id_get();
    recipe.output.count = 1;

    crafting_add_recipe(recipe);
  }
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

