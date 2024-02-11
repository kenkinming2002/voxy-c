#include <world_update.h>

#include <world_update_chunk_generate.h>
#include <world_update_player_spawn.h>
#include <world_update_player_control.h>
#include <world_update_light.h>
#include <world_update_physics.h>
#include <world_update_chunk_mesh.h>

void world_update(struct world *world, struct world_generator *world_generator, struct mod *mod, float dt)
{
  world_update_chunk_generate(world, mod, world_generator);

  world_update_player_spawn  (world, mod);
  world_update_player_control(world, mod, dt);

  world_update_light(world, mod);
  world_update_physics(world, mod, dt);

  world_update_chunk_mesh(world, mod);
}
