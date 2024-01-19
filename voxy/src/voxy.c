#include "camera.h"
#include "window.h"
#include "world.h"
#include "world_generator.h"
#include "world_renderer.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

#define MOVE_SPEED 1.0f
#define PAN_SPEED  0.001f

struct application
{
  struct window          window;
  struct world           world;
  struct world_renderer  world_renderer;
  struct world_generator world_generator;
};

static int application_init(struct application *application)
{
  if(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT) != 0)
    return -1;

  seed_t seed = time(NULL);

  world_init(&application->world, seed);
  world_generator_init(&application->world_generator, seed);
  if(world_renderer_init(&application->world_renderer) != 0)
  {
    window_deinit(&application->window);
    world_deinit(&application->world);
    world_generator_deinit(&application->world_generator);
    return -1;
  }

  return 0;
}

static void application_deinit(struct application *application)
{
  world_deinit(&application->world);
  world_generator_deinit(&application->world_generator);
  world_renderer_deinit(&application->world_renderer);

  window_deinit(&application->window);
}

static void application_update(struct application *application)
{
  world_update(&application->world, &application->window);
  world_generator_update(&application->world_generator, &application->world);
  world_renderer_update(&application->world_renderer, &application->world);
}

static void application_render(struct application *application)
{
  int width, height;
  window_get_framebuffer_size(&application->window, &width, &height);

  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  world_renderer_render(&application->world_renderer, &(struct camera) {
    .transform = application->world.player_transform,
    .fovy      = M_PI / 2.0f,
    .near      = 1.0f,
    .far       = 1000.0f,
    .aspect    = (float)width / (float)height,
  });

  window_swap_buffers(&application->window);
}

static void application_run(struct application *application)
{
  while(!window_should_close(&application->window))
  {
    window_handle_events(&application->window);

    application_update(application);
    application_render(application);
  }
}

int main()
{
  struct application application;
  if(application_init(&application) != 0)
    return EXIT_FAILURE;

  application_run(&application);
  application_deinit(&application);
}
