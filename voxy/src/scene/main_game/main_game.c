#include <voxy/scene/main_game/main_game.h>

#include <voxy/scene/scene.h>

#include <voxy/scene/main_game/entity/player.h>
#include <voxy/scene/main_game/mod.h>
#include <voxy/scene/main_game/render/render.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/chunk_hash_table.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/update/chunk_generate.h>
#include <voxy/scene/main_game/update/chunk_remesh.h>
#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/update/light.h>
#include <voxy/scene/main_game/update/physics.h>

#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>
#include <voxy/core/log.h>
#include <voxy/core/delta_time.h>

#include <glad/glad.h>

#define FIXED_DT (1.0f/30.0f)
#define MOD_PATH "mod/mod.so"

static float accumulated_dt;
static bool initialized;

void main_game_enter()
{
  accumulated_dt = 0.0f;
  if(!initialized)
  {
    initialized = true;
    world_seed_generate();
    mod_load(MOD_PATH);
  }
}

void main_game_leave()
{
}

static void main_game_update_fixed(float dt)
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

  // FIXME: Write a real UI
  if(input_press(KEY_ESC))
    scene_switch(SCENE_MAIN_MENU);
}

void main_game_update(void)
{
  accumulated_dt += get_delta_time();
  if(accumulated_dt >= FIXED_DT)
  {
    accumulated_dt -= FIXED_DT;

    // Window update has to be synchronized with fixed update or otherwise we
    // may get lost input. This is because in our event handling model, key
    // presses/releases are detected by comparing their state before and after
    // event handling in window_update(). Hence, all of them would be lost if
    // we call window_update() again, without handling previous input.
    window_update();
    main_game_update_fixed(FIXED_DT);
  }

  if(accumulated_dt >= FIXED_DT)
  {
    LOG_WARN("Skipping %d fixed update", (int)floorf(accumulated_dt / FIXED_DT));
    accumulated_dt = fmodf(accumulated_dt, FIXED_DT);
  }
}

void main_game_render()
{
  glViewport(0, 0, window_size.x, window_size.y);
  world_render();
  ui_render();
}

