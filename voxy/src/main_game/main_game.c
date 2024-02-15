#include <voxy/main_game/main_game.h>

#include <voxy/main_game/chunk_generate.h>
#include <voxy/main_game/player_spawn.h>
#include <voxy/main_game/player_camera.h>
#include <voxy/main_game/player_movement.h>
#include <voxy/main_game/player_action.h>
#include <voxy/main_game/light.h>
#include <voxy/main_game/physics.h>
#include <voxy/main_game/chunk_remesh.h>
#include <voxy/main_game/ui.h>
#include <voxy/main_game/world_render.h>

#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>

#include <voxy/types/player.h>

#include <voxy/core/window.h>

#include <glad/glad.h>

#include <stdio.h>
#include <stdlib.h>

void main_game_update(float dt)
{
  // 0: Entity Generation? This should be temporary!??
  static float cooldown;
  cooldown += dt;
  if(cooldown >= 2.0f)
  {
    cooldown -= 2.0f;

    struct entity *entity = world_entity_create();
    *entity = player.base;
  }

  // 1: Update World
  update_chunk_generate();
  update_player_spawn();
  update_player_camera();
  update_player_movement(dt);
  update_player_action(dt);
  update_light();
  update_physics(dt);
  update_chunk_remesh();

  // 2: Update UI
  main_game_update_ui();
}

void main_game_render()
{
  glViewport(0, 0, window_size.x, window_size.y);

  world_render();        // 1: Render World
  main_game_render_ui(); // 2: Render UI
}

