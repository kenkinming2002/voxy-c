#include "application.h"
#include "check.h"

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"
#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_init(struct application *application)
{
  CHECK(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT));
  CHECK(application_main_game_init(&application->main_game));
  return 0;
}

void application_fini(struct application *application)
{
  application_main_game_fini(&application->main_game);
  window_fini(&application->window);
}

void application_update(struct application *application, struct input *input, float dt)
{
  application_main_game_update(&application->main_game, input, dt);
}

void application_render(struct application *application, int width, int height)
{
  application_main_game_render(&application->main_game, width, height);

}

void application_run(struct application *application)
{
  double prev_time;
  double next_time;
  double dt;

  prev_time = glfwGetTime();
  while(!window_should_close(&application->window))
  {
    window_handle_events(&application->window);

    next_time = glfwGetTime();
    dt        = next_time - prev_time;
    prev_time = next_time;

    struct input input;
    window_get_input(&application->window, &input);
    application_update(application, &input, dt);

    int width, height;
    window_get_framebuffer_size(&application->window, &width, &height);
    application_render(application, width, height);

    window_swap_buffers(&application->window);
  }
}

