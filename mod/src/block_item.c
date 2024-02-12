#include "ids.h"

#include <voxy/mod_interface.h>
#include <voxy/main_game/world.h>

#include <voxy/types/world.h>
#include <voxy/types/entity.h>

#include <voxy/math/vector.h>

#include <assert.h>
#include <stdint.h>

static uint8_t item_id_to_block_id(uint8_t item_id)
{
  switch(item_id)
  {
  case ITEM_STONE: return BLOCK_STONE;
  case ITEM_GRASS: return BLOCK_GRASS;
  case ITEM_LOG:   return BLOCK_LOG;
  case ITEM_LEAVE: return BLOCK_LEAVE;
  }
  assert(0);
}

void on_block_item_use(uint8_t item_id)
{
  ivec3_t position;
  ivec3_t normal;
  if(world.player.cooldown >= PLAYER_ACTION_COOLDOWN && entity_ray_cast(&world.player.base, &world, 20.0f, &position, &normal))
  {
    world.player.cooldown = 0.0f;
    world_place_block(&world, ivec3_add(position, normal), item_id_to_block_id(item_id));
  }
}

