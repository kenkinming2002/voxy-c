#include "application_main_game.h"

#include <stdio.h>
#include <stdlib.h>

#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  seed_t seed = time(NULL);

  CHECK(resource_pack_load(&application_main_game->resource_pack, RESOURCE_PACK_FILEPATH));

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

void application_main_game_update(struct application_main_game *application_main_game, int width, int height, bool *cursor, struct input *input, float dt)
{
  world_update(&application_main_game->world, &application_main_game->world_generator, &application_main_game->resource_pack, input, dt);
  application_main_game_update_ui(application_main_game, width, height, cursor, input);
}

void application_main_game_render(struct application_main_game *application_main_game, int width, int height, struct renderer_world *renderer_world, struct renderer_ui *renderer_ui)
{
  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_world_render(renderer_world, width, height, &application_main_game->world, &application_main_game->resource_pack);
  application_main_game_render_ui(application_main_game, width, height, renderer_ui);
}

