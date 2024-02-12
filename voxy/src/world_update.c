#include <world_update.h>

#include <main_game/chunk_generate.h>
#include <main_game/player_spawn.h>
#include <main_game/player_camera.h>

#include <world_update_player_control.h>
#include <world_update_light.h>
#include <world_update_physics.h>
#include <world_update_chunk_mesh.h>

void world_update(struct world *world, float dt)
{
  update_chunk_generate();
  update_player_spawn();
  update_player_camera();

  world_update_player_control(world, dt);

  world_update_light(world);
  world_update_physics(world, dt);

  world_update_chunk_mesh(world);
}
