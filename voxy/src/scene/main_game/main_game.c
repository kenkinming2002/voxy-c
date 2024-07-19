#include <voxy/scene/main_game/main_game.h>

#include <voxy/scene/scene.h>

#include <voxy/scene/main_game/mod.h>
#include <voxy/scene/main_game/render/render.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/chunk_hash_table.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/update/chunk_generate.h>
#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/update/light.h>
#include <voxy/scene/main_game/update/physics.h>

#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>
#include <voxy/core/log.h>
#include <voxy/core/delta_time.h>

#include <dlfcn.h>

#include <glad/glad.h>

#define FIXED_DT (1.0f/30.0f)
#define MOD_PATH "mod/mod.so"

static float accumulated_dt;
static bool initialized;

void main_game_enter()
{
  window_show_cursor(false);

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
  window_show_cursor(true);
}

static void main_game_update_fixed(float dt)
{
  ui_reset();

  extern void (*mod_update)(void);
  mod_update();

  // Update World
  {
    update_chunk_generate();

    world_for_each_chunk(chunk)
      if(chunk->data)
        for(size_t i=0; i<chunk->data->entities.item_count; ++i)
        {
          struct entity *entity = &chunk->data->entities.items[i];
          const struct entity_info *entity_info = query_entity_info(entity->id);
          if(entity_info->on_update)
            entity_info->on_update(&chunk->data->entities.items[i], dt);
        }

    world_for_each_chunk(chunk)
      chunk_commit_add_entities(chunk);

    update_light();
    update_physics(dt);
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

