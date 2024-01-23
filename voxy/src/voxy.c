#include "camera.h"
#include "font.h"
#include "ui.h"
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

struct application
{
  struct window          window;
  struct world           world;
  struct world_renderer  world_renderer;
  struct world_generator world_generator;

  struct ui   ui;
  struct font font;
};

static int application_init(struct application *application)
{
  if(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT) != 0)
    goto error1;

  seed_t seed = time(NULL);

  world_init(&application->world, seed);
  world_generator_init(&application->world_generator, seed);
  if(world_renderer_init(&application->world_renderer) != 0)
    goto error2;

  if(ui_init(&application->ui) != 0)
    goto error3;

  if(font_load(&application->font, "assets/arial.ttf") != 0)
    goto error4;

  return 0;

error4:
  ui_deinit(&application->ui);
error3:
  world_renderer_deinit(&application->world_renderer);
error2:
  world_generator_deinit(&application->world_generator);
  world_deinit(&application->world);
  window_deinit(&application->window);
error1:
  return -1;
}

static void application_deinit(struct application *application)
{
  font_unload(&application->font);
  ui_deinit(&application->ui);
  world_renderer_deinit(&application->world_renderer);
  world_generator_deinit(&application->world_generator);
  world_deinit(&application->world);
  window_deinit(&application->window);
}

static void application_update(struct application *application, float dt)
{
  world_update(&application->world, &application->window, dt);
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

  ui_draw_quad(&application->ui, vec2(width, height), vec2(0.0f, 0.0f), vec2(width / 2.0f, height / 2.0f), vec4(1.0f, 1.0f, 0.0f, 0.5f));
  ui_draw_quad_rounded(&application->ui, vec2(width, height), vec2(width / 2.0f, 0.0f), vec2(width / 2.0f, height / 2.0f), 50.0f, vec4(1.0f, 1.0f, 0.0f, 0.5f));
  ui_render_text(&application->ui, &application->font, vec2(width, height), vec2(0.0f, 0.0f),  "hello world");
  ui_render_text(&application->ui, &application->font, vec2(width, height), vec2(0.0f, 48.0f), "goodbye world");
  window_swap_buffers(&application->window);
}

static void application_run(struct application *application)
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

int main()
{
  struct application application;
  if(application_init(&application) != 0)
    return EXIT_FAILURE;

  application_run(&application);
  application_deinit(&application);
}
