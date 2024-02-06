#include "window.h"

#include <stdio.h>

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  fprintf(stderr, "ERROR: %s\n", description);
}

static void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  struct window *_window = glfwGetWindowUserPointer(window);

  if(xoffset > 0.0f) ++_window->mouse_scroll.x;
  if(xoffset < 0.0f) --_window->mouse_scroll.x;

  if(yoffset > 0.0f) ++_window->mouse_scroll.y;
  if(yoffset < 0.0f) --_window->mouse_scroll.y;
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void)scancode;
  (void)mods;

  size_t mask = 0;
  switch(key)
  {
    case GLFW_KEY_1: mask = 1LL << KEY_1; break;
    case GLFW_KEY_2: mask = 1LL << KEY_2; break;
    case GLFW_KEY_3: mask = 1LL << KEY_3; break;
    case GLFW_KEY_4: mask = 1LL << KEY_4; break;
    case GLFW_KEY_5: mask = 1LL << KEY_5; break;
    case GLFW_KEY_6: mask = 1LL << KEY_6; break;
    case GLFW_KEY_7: mask = 1LL << KEY_7; break;
    case GLFW_KEY_8: mask = 1LL << KEY_8; break;
    case GLFW_KEY_9: mask = 1LL << KEY_9; break;

    case GLFW_KEY_KP_1: mask = 1ULL << KEY_KP_1; break;
    case GLFW_KEY_KP_2: mask = 1ULL << KEY_KP_2; break;
    case GLFW_KEY_KP_3: mask = 1ULL << KEY_KP_3; break;
    case GLFW_KEY_KP_4: mask = 1ULL << KEY_KP_4; break;
    case GLFW_KEY_KP_5: mask = 1ULL << KEY_KP_5; break;
    case GLFW_KEY_KP_6: mask = 1ULL << KEY_KP_6; break;
    case GLFW_KEY_KP_7: mask = 1ULL << KEY_KP_7; break;
    case GLFW_KEY_KP_8: mask = 1ULL << KEY_KP_8; break;
    case GLFW_KEY_KP_9: mask = 1ULL << KEY_KP_9; break;

    case GLFW_KEY_A: mask = 1ULL << KEY_A; break;
    case GLFW_KEY_B: mask = 1ULL << KEY_B; break;
    case GLFW_KEY_C: mask = 1ULL << KEY_C; break;
    case GLFW_KEY_D: mask = 1ULL << KEY_D; break;
    case GLFW_KEY_E: mask = 1ULL << KEY_E; break;
    case GLFW_KEY_F: mask = 1ULL << KEY_F; break;
    case GLFW_KEY_G: mask = 1ULL << KEY_G; break;
    case GLFW_KEY_H: mask = 1ULL << KEY_H; break;
    case GLFW_KEY_I: mask = 1ULL << KEY_I; break;
    case GLFW_KEY_J: mask = 1ULL << KEY_J; break;
    case GLFW_KEY_K: mask = 1ULL << KEY_K; break;
    case GLFW_KEY_L: mask = 1ULL << KEY_L; break;
    case GLFW_KEY_M: mask = 1ULL << KEY_M; break;
    case GLFW_KEY_N: mask = 1ULL << KEY_N; break;
    case GLFW_KEY_O: mask = 1ULL << KEY_O; break;
    case GLFW_KEY_P: mask = 1ULL << KEY_P; break;
    case GLFW_KEY_Q: mask = 1ULL << KEY_Q; break;
    case GLFW_KEY_R: mask = 1ULL << KEY_R; break;
    case GLFW_KEY_S: mask = 1ULL << KEY_S; break;
    case GLFW_KEY_T: mask = 1ULL << KEY_T; break;
    case GLFW_KEY_U: mask = 1ULL << KEY_U; break;
    case GLFW_KEY_V: mask = 1ULL << KEY_V; break;
    case GLFW_KEY_W: mask = 1ULL << KEY_W; break;
    case GLFW_KEY_X: mask = 1ULL << KEY_X; break;
    case GLFW_KEY_Y: mask = 1ULL << KEY_Y; break;
    case GLFW_KEY_Z: mask = 1ULL << KEY_Z; break;

    case GLFW_KEY_LEFT_SHIFT:  mask = 1ULL << KEY_SHIFT; break;
    case GLFW_KEY_RIGHT_SHIFT: mask = 1ULL << KEY_SHIFT; break;

    case GLFW_KEY_LEFT_CONTROL:  mask = 1ULL << KEY_CTRL; break;
    case GLFW_KEY_RIGHT_CONTROL: mask = 1ULL << KEY_CTRL; break;

    case GLFW_KEY_SPACE: mask = 1ULL << KEY_SPACE; break;
  }

  struct window *_window = glfwGetWindowUserPointer(window);
  switch(action)
  {
  case GLFW_PRESS:   _window->states |= mask;  break;
  case GLFW_RELEASE: _window->states &= ~mask; break;
  }
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  (void)mods;

  size_t mask = 0;
  switch(button)
  {
    case GLFW_MOUSE_BUTTON_LEFT:   mask = 1ULL << BUTTON_LEFT;   break;
    case GLFW_MOUSE_BUTTON_RIGHT:  mask = 1ULL << BUTTON_RIGHT;  break;
    case GLFW_MOUSE_BUTTON_MIDDLE: mask = 1ULL << BUTTON_MIDDLE; break;
  }

  struct window *_window = glfwGetWindowUserPointer(window);
  switch(action)
  {
  case GLFW_PRESS:   _window->states |= mask;  break;
  case GLFW_RELEASE: _window->states &= ~mask; break;
  }
}

int window_init(struct window *window, const char *title, unsigned width, unsigned height)
{
  // 1: Initialize GLFW
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    fprintf(stderr, "ERROR: Failed to initialize GLFW\n");
    goto error_glfw_init;
  }

  // 2: Create GLFW window
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  if(!(window->window = glfwCreateWindow(width, height, title, NULL, NULL)))
  {
    fprintf(stderr, "ERROR: Failed to create window\n");
    goto error_glfw_window;
  }

  // 3: Initialize OpenGL Context
  glfwMakeContextCurrent(window->window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    fprintf(stderr, "ERROR: Failed to load OpenGL\n");
    goto error_glad;
  }

  // 4: Callback
  glfwSetWindowUserPointer(window->window, window);
  glfwSetScrollCallback(window->window, glfw_scroll_callback);
  glfwSetKeyCallback(window->window, glfw_key_callback);
  glfwSetMouseButtonCallback(window->window, glfw_mouse_button_callback);

  // 5: Setup
  window->states = 0;

  double xpos, ypos;
  glfwSetInputMode(window->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwGetWindowSize(window->window, &window->width, &window->height);
  glfwGetFramebufferSize(window->window, &window->framebuffer_width, &window->framebuffer_height);
  glfwGetCursorPos(window->window, &xpos, &ypos);
  window->mouse_position = fvec2(xpos, window->height - ypos);

  return 0;

error_glad:
  glfwDestroyWindow(window->window);
error_glfw_window:
  glfwTerminate();
error_glfw_init:
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

void window_begin(struct window *window)
{
  size_t  old_states         = window->states;
  fvec2_t old_mouse_position = window->mouse_position;

  window->mouse_scroll = ivec2_zero();

  glfwPollEvents();
  glfwGetWindowSize(window->window, &window->width, &window->height);
  glfwGetFramebufferSize(window->window, &window->framebuffer_width, &window->framebuffer_height);

  window->presses  = ~old_states & window->states;
  window->releases = old_states & ~window->states;

  double xpos, ypos;
  glfwGetCursorPos(window->window, &xpos, &ypos);
  window->mouse_position = fvec2(xpos, window->height - ypos);
  window->mouse_motion   = fvec2_sub(window->mouse_position, old_mouse_position);
}

void window_end(struct window *window)
{
  glfwSwapBuffers(window->window);
}

void window_set_cursor(struct window *window, int cursor)
{
  glfwSetInputMode(window->window, GLFW_CURSOR, cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

