#include "ids.h"

#include <voxy/main_game/entity.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/world.h>

#include <voxy/math/vector.h>

#include <assert.h>
#include <stdint.h>

static block_id_t item_id_to_block_id(item_id_t item_id)
{
  if(item_id == ITEM_ID_STONE) return BLOCK_ID_STONE;
  if(item_id == ITEM_ID_GRASS) return BLOCK_ID_GRASS;
  if(item_id == ITEM_ID_LOG)   return BLOCK_ID_LOG;
  if(item_id == ITEM_ID_LEAVE) return BLOCK_ID_LEAVE;
  assert(0 && "Unreachable");
}

void on_block_item_use(item_id_t item_id)
{
  struct player *player        = player_get();
  struct entity *player_entity = player_as_entity(player);

  ivec3_t position;
  ivec3_t normal;
  if(entity_ray_cast(player_entity, 20.0f, &position, &normal))
    world_block_set(ivec3_add(position, normal), item_id_to_block_id(item_id));
}

