#include <voxy/window.h>

#include <stdio.h>
#include <stdbool.h>

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  fprintf(stderr, "ERROR: %s\n", description);
}

int window_init(struct window *window, const char *title, unsigned width, unsigned height)
{
  bool glfw_initialized = false;
  window->window = NULL;

  // 1: Initialize GLFW
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    fprintf(stderr, "ERROR: Failed to initialize GLFW\n");
    goto error;
  }
  glfw_initialized = true;

  // 2: Create GLFW window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  window->window = glfwCreateWindow(width, height, title, NULL, NULL);
  if(!window->window)
  {
    fprintf(stderr, "ERROR: Failed to create window\n");
    goto error;
  }
  glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwGetCursorPos(window->window, &window->xpos, &window->ypos);

  // 3: Initialize OpenGL Context
  glfwMakeContextCurrent(window->window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    goto error;
  }

  return 0;

error:
  if(glfw_initialized)
    glfwTerminate();

  if(window->window)
    glfwDestroyWindow(window->window);

  return -1;
}

void window_deinit(struct window *window)
{
  glfwDestroyWindow(window->window);
  glfwTerminate();
}

int window_should_close(struct window *window)
{
  return glfwWindowShouldClose(window->window);
}

void window_swap_buffers(struct window *window)
{
  glfwSwapBuffers(window->window);
}

void window_handle_events(struct window *window)
{
  (void)window;
  glfwPollEvents();
}

void window_get_framebuffer_size(struct window *window, int *width, int *height)
{
  glfwGetFramebufferSize(window->window, width, height);
}

void window_get_mouse_motion(struct window *window, float *dx, float *dy)
{
  double new_xpos, new_ypos;
  glfwGetCursorPos(window->window, &new_xpos, &new_ypos);

  *dx = new_xpos - window->xpos;
  *dy = new_ypos - window->ypos;

  window->xpos = new_xpos;
  window->ypos = new_ypos;
}

void window_get_keyboard_motion(struct window *window, float *dx, float *dy, float *dz)
{
  *dx = 0.0f;
  *dy = 0.0f;
  *dz = 0.0f;

  if(glfwGetKey(window->window, GLFW_KEY_D))          *dx += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_A))          *dx -= 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_W))          *dy += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_S))          *dy -= 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_SPACE))      *dz += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_LEFT_SHIFT)) *dz -= 1.0f;
}
