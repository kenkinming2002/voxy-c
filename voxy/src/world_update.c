#include <world_update.h>

#include <main_game/chunk_generate.h>

#include <main_game/player_spawn.h>
#include <main_game/player_camera.h>
#include <main_game/player_movement.h>
#include <main_game/player_action.h>

#include <main_game/light.h>
#include <main_game/physics.h>

#include <main_game/chunk_remesh.h>

void world_update(struct world *world, float dt)
{
  update_chunk_generate();

  update_player_spawn();
  update_player_camera();
  update_player_movement(dt);
  update_player_action(dt);

  update_light();
  update_physics(dt);

  update_chunk_remesh();
}
