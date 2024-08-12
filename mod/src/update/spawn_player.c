#include "spawn_player.h"

#include "../entity/player/player.h"
#include "../generate.h"

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/update/chunk_manager.h>

#include <libcommon/core/log.h>
#include <libcommon/core/fs.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

static bool player_spawned;

void spawn_player_enter(void)
{
  FILE *f = fopen(WORLD_DIRPATH "/mod/base/player_spawned", "rb");
  player_spawned = f;
  if(f)
    fclose(f);
}

void spawn_player_leave(void)
{
}

static void set_player_spawned(void)
{
  char dirpath[] = WORLD_DIRPATH "/mod/base";
  char filepath[] = WORLD_DIRPATH "/mod/base/player_spawned";

  if(mkdir_recursive(dirpath) != 0)
  {
    LOG_ERROR("Failed to save player spawned flag: Failed to create directory: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  FILE *f = fopen(filepath, "wb");
  if(!f)
  {
    LOG_ERROR("Failed to save player spawned flag: Failed to open file");
    exit(EXIT_FAILURE);
  }

  fclose(f);
  player_spawned = true;
}

void spawn_player_update(void)
{
  if(!player_spawned)
  {
    fvec3_t spawn_position = base_generate_player_spawn(world_seed_get());

    // We have a chicken-and-egg problem in which the player is responsible
    // for deciding which chunks is active, but to be able to spawn the player,
    // the chunk in which the player is to be spawned in need to be activated
    // first. Hence we need to activate chunks around the spawn point for the
    // player, and we need to do so even if we manage to spawn the player this
    // frame since player update which will activate chunks around player will
    // only be executed next frame.
    ivec3_t center = fvec3_as_ivec3_floor(fvec3_div_scalar(spawn_position, CHUNK_WIDTH));
    for(int dz = -PLAYER_CHUNK_LOAD_DISTANCE; dz<=PLAYER_CHUNK_LOAD_DISTANCE; ++dz)
      for(int dy = -PLAYER_CHUNK_LOAD_DISTANCE; dy<=PLAYER_CHUNK_LOAD_DISTANCE; ++dy)
        for(int dx = -PLAYER_CHUNK_LOAD_DISTANCE; dx<=PLAYER_CHUNK_LOAD_DISTANCE; ++dx)
          activate_chunk(ivec3_add(center, ivec3(dx, dy, dz)));

    struct entity entity;
    entity_init(&entity, spawn_position, fvec3_zero(), 10.0f, 10.0f);
    player_entity_init(&entity);
    if(!world_add_entity(entity))
      player_entity_fini(&entity);
    else
      set_player_spawned();
  }
}
