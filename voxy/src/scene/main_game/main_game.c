#include <voxy/scene/main_game/main_game.h>

#include <voxy/scene/scene.h>

#include <voxy/scene/main_game/mod.h>
#include <voxy/scene/main_game/render/render.h>
#include <voxy/scene/main_game/render/debug_overlay.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/update/chunk_generate.h>
#include <voxy/scene/main_game/update/chunk_manager.h>
#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/update/light.h>
#include <voxy/scene/main_game/update/physics/physics.h>

#include <libcommon/graphics/ui.h>
#include <libcommon/core/window.h>
#include <libcommon/core/log.h>
#include <libcommon/core/delta_time.h>

#include <dlfcn.h>

#include <glad/glad.h>

#define FIXED_DT (1.0f/30.0f)
#define MOD_PATH "mod/mod.so"

extern void (*mod_enter)(void);
extern void (*mod_leave)(void);
extern void (*mod_update)(void);

static float accumulated_dt;
static bool initialized;

void main_game_enter()
{
  accumulated_dt = 0.0f;
  if(!initialized)
  {
    initialized = true;
    mod_load(MOD_PATH);
  }

  window_show_cursor(false);
  load_world_seed();
  load_active_chunks();
  mod_enter();
}

void main_game_leave()
{
  mod_leave();
  flush_active_chunks();
  save_active_chunks();
  save_world_seed();
  window_show_cursor(true);
}

static void main_game_update_fixed(float dt)
{
  ui_reset();
  main_game_debug_overlay_reset();

  // Update World
  {
    sync_active_chunks();
    reset_active_chunks();
    {
      mod_update();

      world_for_each_chunk(chunk)
        for(size_t i=0; i<chunk->entities.item_count; ++i)
        {
          struct entity *entity = &chunk->entities.items[i];
          const struct entity_info *entity_info = query_entity_info(entity->id);
          if(entity_info->on_update)
            entity_info->on_update(&chunk->entities.items[i], dt);
        }

      update_light();
      update_physics(dt);

      world_for_each_chunk(chunk)
        chunk_commit_add_entities(chunk);

      world_for_each_chunk(chunk)
      {
        size_t new_item_count = 0;
        for(size_t i=0; i<chunk->entities.item_count; ++i)
        {
          struct entity *entity = &chunk->entities.items[i];
          if(!ivec3_eql(chunk->position, get_chunk_position_f(entity->position)) && world_add_entity_raw(*entity))
            continue;

          chunk->entities.items[new_item_count++] = chunk->entities.items[i];
        }
        chunk->entities.item_count = new_item_count;
      }
    }
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

