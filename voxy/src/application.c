#include "application.h"

#include <core/delta_time.h>
#include <main_game/mod.h>

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_init(struct application *application)
{
  window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
  mod_load(MOD_FILEPATH);

  CHECK(application_main_game_init(&application->main_game));
  return 0;
}

void application_fini(struct application *application)
{
  application_main_game_fini(&application->main_game);
}

void application_update(struct application *application, float dt)
{
  application_main_game_update(&application->main_game, dt);
}

void application_render(struct application *application)
{
  application_main_game_render(&application->main_game);
}

void application_run(struct application *application)
{
  while(!window_should_close())
  {
    float dt = get_delta_time();

    window_begin();
    application_update(application, dt);
    application_render(application);
    window_end();
  }
}

