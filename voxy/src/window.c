#include "window.h"

#include <stdio.h>
#include <stdbool.h>

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  fprintf(stderr, "ERROR: %s\n", description);
}

static void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  (void)xoffset;

  struct window *_window = glfwGetWindowUserPointer(window);
  if(yoffset > 0.0f) ++_window->scroll;
  if(yoffset < 0.0f) --_window->scroll;
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  (void)mods;

  struct window *_window = glfwGetWindowUserPointer(window);
  if(action == GLFW_PRESS)
    switch(button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:  _window->click_left  += 1; break;
    case GLFW_MOUSE_BUTTON_RIGHT: _window->click_right += 1; break;
    }
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void)scancode;
  (void)mods;

  struct window *_window = glfwGetWindowUserPointer(window);
  if(action == GLFW_PRESS)
    switch(key)
    {
    case GLFW_KEY_I: _window->click_i += 1; break;
    }
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
  window->mouse_position = window_get_mouse_position(window);

  // 3: Initialize OpenGL Context
  glfwMakeContextCurrent(window->window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    goto error;
  }

  // 4: Callback
  glfwSetWindowUserPointer(window->window, window);
  glfwSetScrollCallback(window->window, glfw_scroll_callback);
  glfwSetMouseButtonCallback(window->window, glfw_mouse_button_callback);
  glfwSetKeyCallback(window->window, glfw_key_callback);

  return 0;

error:
  if(glfw_initialized)
    glfwTerminate();

  if(window->window)
    glfwDestroyWindow(window->window);

  return -1;
}

void window_fini(struct window *window)
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

void window_get_input(struct window *window, struct input *input)
{
  input->mouse_motion    = window_get_mouse_motion(window);
  input->keyboard_motion = window_get_keyboard_motion(window);

  input->selects[0] = glfwGetKey(window->window, GLFW_KEY_1) || glfwGetKey(window->window, GLFW_KEY_KP_1);
  input->selects[1] = glfwGetKey(window->window, GLFW_KEY_2) || glfwGetKey(window->window, GLFW_KEY_KP_2);
  input->selects[2] = glfwGetKey(window->window, GLFW_KEY_3) || glfwGetKey(window->window, GLFW_KEY_KP_3);
  input->selects[3] = glfwGetKey(window->window, GLFW_KEY_4) || glfwGetKey(window->window, GLFW_KEY_KP_4);
  input->selects[4] = glfwGetKey(window->window, GLFW_KEY_5) || glfwGetKey(window->window, GLFW_KEY_KP_5);
  input->selects[5] = glfwGetKey(window->window, GLFW_KEY_6) || glfwGetKey(window->window, GLFW_KEY_KP_6);
  input->selects[6] = glfwGetKey(window->window, GLFW_KEY_7) || glfwGetKey(window->window, GLFW_KEY_KP_7);
  input->selects[7] = glfwGetKey(window->window, GLFW_KEY_8) || glfwGetKey(window->window, GLFW_KEY_KP_8);
  input->selects[8] = glfwGetKey(window->window, GLFW_KEY_9) || glfwGetKey(window->window, GLFW_KEY_KP_9);

  input->scroll      = window->scroll;      window->scroll      = 0;
  input->click_left  = window->click_left;  window->click_left  = 0;
  input->click_right = window->click_right; window->click_right = 0;
  input->click_i     = window->click_i;     window->click_i     = 0;

  input->state_left  = glfwGetMouseButton(window->window, GLFW_MOUSE_BUTTON_LEFT);
  input->state_right = glfwGetMouseButton(window->window, GLFW_MOUSE_BUTTON_RIGHT);
  input->state_ctrl  = glfwGetKey(window->window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window->window, GLFW_KEY_RIGHT_CONTROL);
}

fvec2_t window_get_mouse_position(struct window *window)
{
  double x, y;
  glfwGetCursorPos(window->window, &x, &y);
  return fvec2(x, y);
}

fvec2_t window_get_mouse_motion(struct window *window)
{
  fvec2_t mouse_position_new = window_get_mouse_position(window);
  fvec2_t mouse_motion = fvec2_sub(mouse_position_new, window->mouse_position);
  window->mouse_position = mouse_position_new;
  return mouse_motion;
}

fvec3_t window_get_keyboard_motion(struct window *window)
{
  fvec3_t keyboard_motion = fvec3_zero();;

  if(glfwGetKey(window->window, GLFW_KEY_D))          keyboard_motion.x += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_A))          keyboard_motion.x -= 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_W))          keyboard_motion.y += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_S))          keyboard_motion.y -= 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_SPACE))      keyboard_motion.z += 1.0f;
  if(glfwGetKey(window->window, GLFW_KEY_LEFT_SHIFT)) keyboard_motion.z -= 1.0f;

  return fvec3_normalize(keyboard_motion);
}

int window_get_key(struct window *window, int key)
{
  return glfwGetKey(window->window, key);
}

