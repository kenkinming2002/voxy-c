#include "spawn_player.h"

#include "../entity/player/player.h"
#include "../generate.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/update/chunk_generate.h>

#include <stdbool.h>

void update_spawn_player(void)
{
  static bool player_spawned = false;
  if(!player_spawned)
  {
    struct entity entity;
    entity.position = base_generate_player_spawn(world_seed_get());
    entity.velocity = fvec3_zero();
    entity.rotation = fvec3_zero();
    entity.grounded = false;
    player_entity_init(&entity);
    if(!world_add_entity(entity))
    {
      player_entity_fini(&entity);

      // We have a chicken-and-egg problem in which the player is responsible
      // for deciding which chunks to load, but to be able to spawn the player,
      // the chunk in which the player is to be spawned in need to be loaded
      // first.
      ivec3_t center = fvec3_as_ivec3_floor(fvec3_div_scalar(entity.position, CHUNK_WIDTH));
      for(int dz = -PLAYER_CHUNK_LOAD_DISTANCE; dz<=PLAYER_CHUNK_LOAD_DISTANCE; ++dz)
        for(int dy = -PLAYER_CHUNK_LOAD_DISTANCE; dy<=PLAYER_CHUNK_LOAD_DISTANCE; ++dy)
          for(int dx = -PLAYER_CHUNK_LOAD_DISTANCE; dx<=PLAYER_CHUNK_LOAD_DISTANCE; ++dx)
            enqueue_chunk_generate(ivec3_add(center, ivec3(dx, dy, dz)));
    }
    else
      player_spawned = true;
  }
}
