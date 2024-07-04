#include <voxy/main_game/main_game.h>

#include <voxy/main_game/types/chunk.h>
#include <voxy/main_game/types/chunk_hash_table.h>
#include <voxy/main_game/types/entity.h>

#include <voxy/main_game/states/chunks.h>
#include <voxy/main_game/states/seed.h>

#include <voxy/main_game/update/chunk_generate.h>
#include <voxy/main_game/update/chunk_remesh.h>
#include <voxy/main_game/update/generate.h>
#include <voxy/main_game/update/light.h>
#include <voxy/main_game/update/physics.h>

#include <voxy/main_game/render/render.h>

#include <voxy/main_game/entity/player.h>

#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>
#include <voxy/core/log.h>

#include <glad/glad.h>

void main_game_update(float dt)
{
  ui_reset();

  // Player Spawning
  static bool player_spawned = false;
  if(!player_spawned)
  {
    struct entity entity;
    entity.position = generate_player_spawn(world_seed_get());
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
      for(int dz = -GENERATOR_DISTANCE_PLAYER; dz<=GENERATOR_DISTANCE_PLAYER; ++dz)
        for(int dy = -GENERATOR_DISTANCE_PLAYER; dy<=GENERATOR_DISTANCE_PLAYER; ++dy)
          for(int dx = -GENERATOR_DISTANCE_PLAYER; dx<=GENERATOR_DISTANCE_PLAYER; ++dx)
            enqueue_chunk_generate(ivec3_add(center, ivec3(dx, dy, dz)));
    }
    else
      player_spawned = true;
  }

  // Update World
  {
    update_chunk_generate();
    world_for_each_chunk(chunk)
      if(chunk->data)
        for(size_t i=0; i<chunk->data->entity_count; ++i)
        {
          struct entity *entity = &chunk->data->entities[i];
          const struct entity_info *entity_info = query_entity_info(entity->id);
          if(entity_info->on_update)
            entity_info->on_update(&chunk->data->entities[i], dt);
        }

    update_light();
    update_physics(dt);
    update_chunk_remesh();
  }
}

void main_game_render()
{
  glViewport(0, 0, window_size.x, window_size.y);
  world_render();
  ui_render();
}

