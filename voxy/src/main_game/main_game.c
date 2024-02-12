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

#include <voxy/core/window.h>

#include <glad/glad.h>

void main_game_update(float dt)
{
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
  // 1: Render World
  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  world_render();

  // 2: Render UI
  main_game_render_ui();
}

