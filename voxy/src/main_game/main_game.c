#include <voxy/main_game/main_game.h>

#include <voxy/main_game/types/chunk.h>
#include <voxy/main_game/types/entity.h>

#include <voxy/main_game/states/chunk_hash_table.h>
#include <voxy/main_game/states/world.h>
#include <voxy/main_game/states/world_seed.h>

#include <voxy/main_game/update/generate.h>
#include <voxy/main_game/update/chunk_generate.h>
#include <voxy/main_game/update/light.h>
#include <voxy/main_game/update/physics.h>

#include <voxy/main_game/render/chunk_remesh.h>
#include <voxy/main_game/render/world_render.h>

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
    if(world_entity_add(entity) != 0)
      player_entity_fini(&entity);
    else
      player_spawned = true;
  }

  // Update World
  {
    update_chunk_generate();
    world_chunk_for_each(chunk)
      for(size_t i=0; i<chunk->entity_count; ++i)
      {
        struct entity *entity = &chunk->entities[i];
        const struct entity_info *entity_info = query_entity_info(entity->id);
        if(entity_info->on_update)
          entity_info->on_update(&chunk->entities[i], dt);
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

