#include "camera.h"
#include "font_set.h"
#include "renderer.h"
#include "ui.h"
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
#define RESOURCE_PACK_FILEPATH "resource_pack/resource_pack.so"

#define CHECK(expr) if((expr) != 0) { fprintf(stderr, "%s:%d: ERROR: %s != 0\n", __FILE__, __LINE__, #expr); exit(EXIT_FAILURE); }

struct application
{
  struct resource_pack resource_pack;

  struct window   window;
  struct renderer renderer;

  struct ui       ui;
  struct font_set font_set;

  struct world           world;
  struct world_generator world_generator;

  int selection;
};

static void application_on_scroll(GLFWwindow *window, double xoffset, double offset);

static int application_init(struct application *application)
{
  seed_t seed = time(NULL);

  CHECK(resource_pack_load(&application->resource_pack, RESOURCE_PACK_FILEPATH));

  CHECK(window_init(&application->window, WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT));
  CHECK(renderer_init(&application->renderer, &application->resource_pack));
  CHECK(ui_init(&application->ui));

  font_set_init(&application->font_set);
  font_set_load(&application->font_set, "assets/arial.ttf");
  font_set_load(&application->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&application->font_set);

  world_init(&application->world, seed);
  world_generator_init(&application->world_generator, seed);

  glfwSetWindowUserPointer(application->window.window, application);
  glfwSetScrollCallback(application->window.window, application_on_scroll);

  return 0;
}

static void application_fini(struct application *application)
{
  world_generator_fini(&application->world_generator);
  world_fini(&application->world);

  font_set_fini(&application->font_set);

  ui_fini(&application->ui);
  renderer_fini(&application->renderer);
  window_fini(&application->window);

  resource_pack_unload(&application->resource_pack);
}

static void application_on_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
  (void)xoffset;

  struct application *application = glfwGetWindowUserPointer(window);
  if(yoffset > 0.0f) ++application->selection;
  if(yoffset < 0.0f) --application->selection;
  application->selection -= 1;
  application->selection += 9;
  application->selection %= 9;
  application->selection += 1;
}

static void application_update(struct application *application, float dt)
{
  struct input input;
  window_get_input(&application->window, &input);
  for(unsigned i=1; i<=9; ++i)
    if(input.selects[i-1])
      application->selection = i;

  world_update(&application->world, &application->world_generator,  &application->resource_pack,&input, dt);
}

static inline float minf(float a, float b)
{
  return a < b ? a : b;
}

static void application_render(struct application *application)
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

  ui_begin(&application->ui, fvec2(width, height));

  const int count = 9;

  const float sep               = minf(width, height) * 0.006f;
  const float inner_width       = minf(width, height) * 0.05f;
  const float outer_width       = inner_width + 2.0f * sep;
  const float total_width       = count * inner_width + (count + 1) * sep;
  const float total_height      = outer_width;
  const float margin_horizontal = (width - total_width) * 0.5f;
  const float margin_vertical   = height * 0.03f;

  char buffer[32];

  snprintf(buffer, sizeof buffer, "Selected %d 你好 😀", application->selection);
  ui_draw_text_centered(&application->ui, &application->font_set, fvec2(width * 0.5f, margin_vertical + outer_width + sep), buffer, 24);
  ui_draw_quad_rounded(&application->ui, fvec2(margin_horizontal, margin_vertical), fvec2(total_width, total_height), sep, fvec4(0.9f, 0.9f, 0.9f, 0.3f));
  for(int i=0; i<count; ++i)
  {
    fvec4_t color = i + 1 == application->selection ? fvec4(0.95f, 0.75f, 0.75f, 0.8f) : fvec4(0.95f, 0.95f, 0.95f, 0.7f);
    ui_draw_quad_rounded(&application->ui, fvec2(margin_horizontal + i * inner_width + (i + 1) * sep, margin_vertical + sep), fvec2(inner_width, inner_width), sep, color);
  }

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
  application_fini(&application);
}
