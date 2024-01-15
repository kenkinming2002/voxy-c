#include "camera.h"
#include "renderer.h"
#include "window.h"
#include "world.h"
#include "world_generator.h"

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
  struct renderer        renderer;
  struct world           world;
  struct world_generator world_generator;
};

static int application_init(struct application *application)
{
  bool window_initialized   = false;
  bool renderer_initialized = false;

  if(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT) != 0)
    goto error;
  window_initialized = true;

  if(renderer_init(&application->renderer) != 0)
    goto error;
  renderer_initialized = true;

  world_init(&application->world, time(NULL));
  world_generator_init(&application->world_generator);
  return 0;

error:
  if(renderer_initialized)
    renderer_deinit(&application->renderer);

  if(window_initialized)
    window_deinit(&application->window);

  return -1;
}

static void application_deinit(struct application *application)
{
  world_deinit(&application->world);
  world_generator_deinit(&application->world_generator);

  renderer_deinit(&application->renderer);
  window_deinit(&application->window);
}

static void application_update(struct application *application)
{
  world_update(&application->world, &application->window);
  world_generator_update(&application->world_generator, &application->world);
}

static void application_render(struct application *application)
{
  int width, height;
  window_get_framebuffer_size(&application->window, &width, &height);

  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_update(&application->renderer, &application->world);
  renderer_render(&application->renderer, &(struct camera) {
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
