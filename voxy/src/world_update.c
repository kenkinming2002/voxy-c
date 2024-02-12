#include <world_update.h>

#include <main_game/chunk_generate.h>

#include <world_update_player_spawn.h>
#include <world_update_player_control.h>
#include <world_update_light.h>
#include <world_update_physics.h>
#include <world_update_chunk_mesh.h>

void world_update(struct world *world, float dt)
{
  update_chunk_generate();

  world_update_player_spawn  (world);
  world_update_player_control(world, dt);

  world_update_light(world);
  world_update_physics(world, dt);

  world_update_chunk_mesh(world);
}
