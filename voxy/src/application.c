#include "application.h"
#include "check.h"

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"
#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

int application_init(struct application *application)
{
  seed_t seed = time(NULL);

  CHECK(resource_pack_load(&application->resource_pack, RESOURCE_PACK_FILEPATH));

  CHECK(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT));
  CHECK(renderer_init(&application->renderer, &application->resource_pack));

  world_init(&application->world, seed);
  world_generator_init(&application->world_generator, seed);

  return 0;
}

void application_fini(struct application *application)
{
  world_generator_fini(&application->world_generator);
  world_fini(&application->world);

  renderer_fini(&application->renderer);
  window_fini(&application->window);

  resource_pack_unload(&application->resource_pack);
}

void application_update(struct application *application, float dt)
{
  struct input input;
  window_get_input(&application->window, &input);
  world_update(&application->world, &application->world_generator, &application->resource_pack, &input, dt);
}

void application_render(struct application *application)
{
  int width, height;
  window_get_framebuffer_size(&application->window, &width, &height);
  renderer_render(&application->renderer, &application->window, &(struct camera) {
    .transform = application->world.player.transform,
    .fovy      = M_PI / 2.0f,
    .near      = 1.0f,
    .far       = 1000.0f,
    .aspect    = (float)width / (float)height,
  }, &application->world);
  window_swap_buffers(&application->window);

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

    application_update(application, dt);
    application_render(application);
  }
}

