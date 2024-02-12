#include "application_main_game.h"

#include <main_game/world.h>

#include <world_update.h>
#include <world_render.h>

#include <core/window.h>

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  CHECK(mod_load(&application_main_game->mod, MOD_FILEPATH));
  CHECK(mod_assets_load(&application_main_game->mod_assets, &application_main_game->mod));

  world_generator_init(&application_main_game->world_generator);
  return 0;
}

void application_main_game_fini(struct application_main_game *application_main_game)
{
  world_generator_fini(&application_main_game->world_generator);

  mod_assets_unload(&application_main_game->mod_assets);
  mod_unload(&application_main_game->mod);
}

void application_main_game_update(struct application_main_game *application_main_game, float dt)
{
  struct world *world = main_game_world_get();
  world_update(world, &application_main_game->world_generator, &application_main_game->mod, dt);
  application_main_game_update_ui(application_main_game);
}

void application_main_game_render(struct application_main_game *application_main_game)
{
  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  struct world *world = main_game_world_get();
  world_render(world, &application_main_game->mod, &application_main_game->mod_assets);
  application_main_game_render_ui(application_main_game);
}

