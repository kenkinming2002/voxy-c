#include "application_main_game.h"

#include <main_game/world.h>
#include <main_game/chunk_generate.h>
#include <main_game/ui.h>
#include <main_game/world_render.h>

#include <world_update.h>

#include <core/window.h>

#include <glad/glad.h>

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  return 0;
}

void application_main_game_fini(struct application_main_game *application_main_game)
{
}

void application_main_game_update(struct application_main_game *application_main_game, float dt)
{
  world_update(&world, dt);
  main_game_update_ui();
}

void application_main_game_render(struct application_main_game *application_main_game)
{
  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  world_render();
  main_game_render_ui();
}

