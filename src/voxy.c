#include <voxy/world.h>
#include <voxy/camera.h>
#include <voxy/renderer.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

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
  GLFWwindow *window;
  double xpos, ypos;

  struct renderer renderer;
  struct world    world;
  struct camera   camera;
};

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  fprintf(stderr, "ERROR: %s\n", description);
}

static int application_init(struct application *application)
{
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    fprintf(stderr, "ERROR: Failed to initialize GLFW\n");
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  application->window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
  if(!application->window)
  {
    fprintf(stderr, "ERROR: Failed to create window\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(application->window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    glfwDestroyWindow(application->window);
    glfwTerminate();
    return EXIT_FAILURE;
  }

  glfwSetInputMode(application->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwGetCursorPos(application->window, &application->xpos, &application->ypos);

  if(renderer_init(&application->renderer) != 0)
    return -1;

  world_init(&application->world);

  struct chunk chunk;

  chunk.x = 0;
  chunk.y = 0;
  chunk.z = 0;
  chunk_randomize(&chunk);
  world_chunk_add(&application->world, chunk);

  chunk.x = 1;
  chunk.y = 0;
  chunk.z = 0;
  chunk_randomize(&chunk);
  world_chunk_add(&application->world, chunk);

  application->camera.transform.translation = vec3(10.0f, -10.0f, 0.0f);
  application->camera.transform.rotation    = vec3(0.0f, 0.0f, 0.0f);
  application->camera.fovy                  = M_PI / 2.0f;
  application->camera.aspect                = 1.0f;
  application->camera.near                  = 1.0f;
  application->camera.far                   = 1000.0f;

  return 0;
}

static void application_deinit(struct application *application)
{
  glfwDestroyWindow(application->window);
  glfwTerminate();
}

static void application_get_mouse_motion(struct application *application, float *dx, float *dy)
{
  double new_xpos, new_ypos;
  glfwGetCursorPos(application->window, &new_xpos, &new_ypos);

  *dx = new_xpos - application->xpos;
  *dy = new_ypos - application->ypos;

  application->xpos = new_xpos;
  application->ypos = new_ypos;
}

static void application_get_keyboard_motion(struct application *application, float *dx, float *dy, float *dz)
{
  *dx = 0.0f;
  *dy = 0.0f;
  *dz = 0.0f;

  if(glfwGetKey(application->window, GLFW_KEY_D))          *dx += 1.0f;
  if(glfwGetKey(application->window, GLFW_KEY_A))          *dx -= 1.0f;
  if(glfwGetKey(application->window, GLFW_KEY_W))          *dy += 1.0f;
  if(glfwGetKey(application->window, GLFW_KEY_S))          *dy -= 1.0f;
  if(glfwGetKey(application->window, GLFW_KEY_SPACE))      *dz += 1.0f;
  if(glfwGetKey(application->window, GLFW_KEY_LEFT_SHIFT)) *dz -= 1.0f;
}

static void application_run(struct application *application)
{
  while(!glfwWindowShouldClose(application->window))
  {
    int width, height;
    glfwGetFramebufferSize(application->window, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderer_update(&application->renderer, &application->world);
    renderer_render(&application->renderer, &application->camera);

    glfwSwapBuffers(application->window);
    glfwPollEvents();

    application->camera.aspect = (float)width / (float)height;

    struct vec3 rotation    = vec3_zero();
    struct vec3 translation = vec3_zero();

    application_get_mouse_motion   (application, &rotation.yaw, &rotation.pitch);
    application_get_keyboard_motion(application, &translation.x, &translation.y, &translation.z);

    rotation = vec3_mul(rotation, PAN_SPEED);
    translation = vec3_normalize(translation);
    translation = vec3_mul(translation, MOVE_SPEED);

    transform_rotate(&application->camera.transform, rotation);
    transform_local_translate(&application->camera.transform, translation);
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
