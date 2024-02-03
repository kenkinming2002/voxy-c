#include "application_main_game.h"

#include <stdio.h>
#include <stdlib.h>

#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_main_game_init(struct application_main_game *application_main_game)
{
  seed_t seed = time(NULL);

  CHECK(resource_pack_load(&application_main_game->resource_pack, RESOURCE_PACK_FILEPATH));
  CHECK(renderer_init(&application_main_game->renderer));

  world_init(&application_main_game->world, seed);
  world_generator_init(&application_main_game->world_generator, seed);
  return 0;
}

void application_main_game_fini(struct application_main_game *application_main_game)
{
  world_generator_fini(&application_main_game->world_generator);
  world_fini(&application_main_game->world);

  renderer_fini(&application_main_game->renderer);
  resource_pack_unload(&application_main_game->resource_pack);
}

void application_main_game_update(struct application_main_game *application_main_game, struct input *input, float dt)
{
  world_update(&application_main_game->world, &application_main_game->world_generator, &application_main_game->resource_pack, input, dt);
}

void application_main_game_render(struct application_main_game *application_main_game, int width, int height)
{
  struct camera camera;
  camera.transform = application_main_game->world.player.transform;
  camera.fovy      = M_PI / 2.0f;
  camera.near      = 1.0f;
  camera.far       = 1000.0f;
  camera.aspect    = (float)width / (float)height;
  renderer_render(&application_main_game->renderer, width, height, &application_main_game->resource_pack, &camera, &application_main_game->world);
}

