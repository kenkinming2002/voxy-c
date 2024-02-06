#include "application_main_game.h"

#include "window.h"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  if(resource_pack_load(&application_main_game->resource_pack, RESOURCE_PACK_FILEPATH) != 0)
    return -1;

  seed_t seed = time(NULL);
  world_init(&application_main_game->world, seed);
  world_generator_init(&application_main_game->world_generator, seed);
  return 0;
}

void application_main_game_fini(struct application_main_game *application_main_game)
{
  world_generator_fini(&application_main_game->world_generator);
  world_fini(&application_main_game->world);
  resource_pack_unload(&application_main_game->resource_pack);
}

void application_main_game_update(struct application_main_game *application_main_game, struct window *window, float dt)
{
  application_main_game_update_world(application_main_game, window, dt);
  application_main_game_update_ui   (application_main_game, window);
}

void application_main_game_render(struct application_main_game *application_main_game, struct window *window, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui)
{
  glViewport(0, 0, window->width, window->height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  application_main_game_render_world(application_main_game, window, renderer_world);
  application_main_game_render_ui   (application_main_game, window, renderer_ui);
}

