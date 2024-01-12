#include <voxy/window.h>
#include <voxy/world.h>
#include <voxy/camera.h>
#include <voxy/renderer.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

#define MOVE_SPEED 1.0f
#define PAN_SPEED  0.001f

struct application
{
  struct window   window;
  struct renderer renderer;
  struct world    world;
  struct camera   camera;
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

  world_init(&application->world);
  {
    struct chunk chunk;
    chunk.x = 0;
    chunk.y = 0;
    chunk.z = 0;
    chunk_randomize(&chunk);
    world_chunk_add(&application->world, chunk);
  }
  {
    struct chunk chunk;
    chunk.x = 1;
    chunk.y = 0;
    chunk.z = 0;
    chunk_randomize(&chunk);
    world_chunk_add(&application->world, chunk);
  }

  application->camera.transform.translation = vec3(10.0f, -10.0f, 0.0f);
  application->camera.transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  application->camera.fovy                  = M_PI / 2.0f;
  application->camera.aspect                = 1.0f;
  application->camera.near                  = 1.0f;
  application->camera.far                   = 1000.0f;
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
  renderer_deinit(&application->renderer);
  window_deinit(&application->window);
}

static void application_update(struct application *application)
{
  struct vec3 rotation = vec3_zero();
  struct vec3 translation = vec3_zero();

  window_get_mouse_motion(&application->window, &rotation.yaw, &rotation.pitch);
  window_get_keyboard_motion(&application->window, &translation.x, &translation.y, &translation.z);

  rotation    = vec3_mul(rotation, PAN_SPEED);
  translation = vec3_normalize(translation);
  translation = vec3_mul(translation, MOVE_SPEED);

  transform_rotate(&application->camera.transform, rotation);
  transform_local_translate(&application->camera.transform, translation);
}

static void application_render(struct application *application)
{
  int width, height;
  window_get_framebuffer_size(&application->window, &width, &height);

  glViewport(0, 0, width, height);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  application->camera.aspect = (float)width / (float)height;

  renderer_update(&application->renderer, &application->world);
  renderer_render(&application->renderer, &application->camera);

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
