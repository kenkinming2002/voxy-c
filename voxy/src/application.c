#include "application.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_init(struct application *application)
{
  window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

  CHECK(application_main_game_init(&application->main_game));
  CHECK(renderer_world_init(&application->renderer_world));
  CHECK(renderer_ui_init(&application->renderer_ui));
  return 0;
}

void application_fini(struct application *application)
{
  renderer_ui_fini(&application->renderer_ui);
  renderer_world_fini(&application->renderer_world);
  application_main_game_fini(&application->main_game);
}

void application_update(struct application *application, float dt)
{
  application_main_game_update(&application->main_game, dt);
}

void application_render(struct application *application)
{
  application_main_game_render(&application->main_game, &application->renderer_world, &application->renderer_ui);
}

void application_run(struct application *application)
{
  double prev_time;
  double next_time;
  double dt;

  prev_time = glfwGetTime();
  while(!window_should_close())
  {
    next_time = glfwGetTime();
    dt        = next_time - prev_time;
    prev_time = next_time;

    window_begin();
    application_update(application, dt);
    application_render(application);
    window_end();
  }
}

