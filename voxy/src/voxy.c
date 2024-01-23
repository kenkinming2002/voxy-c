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
#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

struct application
{
  struct world           world;
  struct world_generator world_generator;

  struct window          window;
  struct resource_pack   resource_pack;
  struct world_renderer  world_renderer;

  struct ui   ui;
  struct font font;
};

static int application_init(struct application *application)
{
  bool window_initialized         = false;
  bool resource_pack_loaded       = false;
  bool world_renderer_initialized = false;
  bool ui_initialized             = false;
  bool font_loaded                = false;

  if(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT)   != 0) goto error; else window_initialized         = true;
  if(resource_pack_load(&application->resource_pack, RESOURCE_PACK_FILEPATH)        != 0) goto error; else resource_pack_loaded       = true;
  if(world_renderer_init(&application->world_renderer, &application->resource_pack) != 0) goto error; else world_renderer_initialized = true;
  if(ui_init(&application->ui)                                                      != 0) goto error; else ui_initialized             = true;
  if(font_load(&application->font, "assets/arial.ttf")                              != 0) goto error; else font_loaded                = true;

  seed_t seed = time(NULL);
  world_init(&application->world, seed);
  world_generator_init(&application->world_generator, seed);
  return 0;

error:
  if(font_loaded)                font_unload(&application->font);
  if(ui_initialized)             ui_deinit(&application->ui);
  if(world_renderer_initialized) world_renderer_deinit(&application->world_renderer);
  if(resource_pack_loaded)       resource_pack_unload(&application->resource_pack);
  if(window_initialized)         window_deinit(&application->window);
  return -1;
}

static void application_deinit(struct application *application)
{
  world_generator_deinit(&application->world_generator);
  world_deinit(&application->world);

  font_unload(&application->font);
  ui_deinit(&application->ui);
  world_renderer_deinit(&application->world_renderer);
  resource_pack_unload(&application->resource_pack);
  window_deinit(&application->window);
}

static void application_update(struct application *application, float dt)
{
  world_update(&application->world, &application->window, dt);
  world_generator_update(&application->world_generator, &application->world);
  world_renderer_update(&application->world_renderer, &application->resource_pack, &application->world);
}

static inline float minf(float a, float b)
{
  return a < b ? a : b;
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

  ui_begin(&application->ui, vec2(width, height));

  const size_t count = 15;

  const float sep               = minf(width, height) * 0.006f;
  const float inner_width       = minf(width, height) * 0.05f;
  const float outer_width       = inner_width + 2.0f * sep;
  const float total_width       = count * inner_width + (count + 1) * sep;
  const float total_height      = outer_width;
  const float margin_horizontal = (width - total_width) * 0.5f;
  const float margin_vertical   = height * 0.03f;

  ui_draw_text_centered(&application->ui, &application->font, vec2(width * 0.5f, margin_vertical + outer_width + sep), "Hello World");
  ui_draw_quad_rounded(&application->ui, vec2(margin_horizontal, margin_vertical), vec2(total_width, total_height), sep, vec4(0.9f, 0.9f, 0.9f, 0.3f));
  for(size_t i=0; i<count; ++i)
    ui_draw_quad_rounded(&application->ui, vec2(margin_horizontal + i * inner_width + (i + 1) * sep, margin_vertical + sep), vec2(inner_width, inner_width), sep, vec4(0.95f, 0.95f, 0.95f, 0.7f));

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
