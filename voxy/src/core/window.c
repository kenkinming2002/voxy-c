#include <voxy/core/window.h>

#include <voxy/core/log.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <stdlib.h>

static GLFWwindow *window;

ivec2_t window_size;
ivec2_t framebuffer_size;

size_t input_states;
size_t input_presses;
size_t input_releases;

static bool mouse_position_initialized;

fvec2_t mouse_position;
fvec2_t mouse_motion;
ivec2_t mouse_scroll;

static void glfw_error_callback(int error, const char *description)
{
  (void)error;
  LOG_WARN("glfw: %s", description);
}

static void glfw_window_size_callback(GLFWwindow *window, int width, int height)
{
  (void)window;
  window_size = ivec2(width, height);
}

static void glfw_framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  (void)window;
  framebuffer_size = ivec2(width, height);
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
  (void)window;
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

  switch(action)
  {
  case GLFW_PRESS:   input_states |= mask;  break;
  case GLFW_RELEASE: input_states &= ~mask; break;
  }
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
  (void)window;
  (void)mods;

  size_t mask = 0;
  switch(button)
  {
    case GLFW_MOUSE_BUTTON_LEFT:   mask = 1ULL << BUTTON_LEFT;   break;
    case GLFW_MOUSE_BUTTON_RIGHT:  mask = 1ULL << BUTTON_RIGHT;  break;
    case GLFW_MOUSE_BUTTON_MIDDLE: mask = 1ULL << BUTTON_MIDDLE; break;
  }

  switch(action)
  {
  case GLFW_PRESS:   input_states |= mask;  break;
  case GLFW_RELEASE: input_states &= ~mask; break;
  }
}

static void glfw_cursor_pos_callback(GLFWwindow *window, double xpos, double ypos)
{
  (void)window;
  mouse_position_initialized = true;
  mouse_position = fvec2(xpos, window_size.y - ypos);
}

static void glfw_scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
  (void)window;

  if(xoffset > 0.0f) ++mouse_scroll.x;
  if(xoffset < 0.0f) --mouse_scroll.x;

  if(yoffset > 0.0f) ++mouse_scroll.y;
  if(yoffset < 0.0f) --mouse_scroll.y;
}

static void window_atexit(void)
{
  glfwDestroyWindow(window);
  glfwTerminate();
}

void window_init(const char *title, unsigned width, unsigned height)
{
  glfwSetErrorCallback(&glfw_error_callback);
  if(!glfwInit())
  {
    LOG_ERROR("Failed to initialize GLFW");
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
  if(!(window = glfwCreateWindow(width, height, title, NULL, NULL)))
  {
    LOG_ERROR("Failed to create window");
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    LOG_ERROR("Failed to load OpenGL");
    exit(EXIT_FAILURE);
  }

  glfwSetWindowSizeCallback(window, glfw_window_size_callback);
  glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
  glfwSetKeyCallback(window, glfw_key_callback);
  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetCursorPosCallback(window, glfw_cursor_pos_callback);
  glfwSetScrollCallback(window, glfw_scroll_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  atexit(window_atexit);
}

bool window_should_close()
{
  return glfwWindowShouldClose(window);
}

void window_update()
{
  size_t  old_input_states;
  bool    old_mouse_position_initialized;
  fvec2_t old_mouse_position;

  old_input_states               = input_states;
  old_mouse_position_initialized = mouse_position_initialized;
  old_mouse_position             = mouse_position;
  mouse_scroll                   = ivec2_zero();

  glfwPollEvents();

  input_presses  = ~old_input_states & input_states;
  input_releases = old_input_states & ~input_states;
  if(old_mouse_position_initialized)
    mouse_motion = fvec2_sub(mouse_position, old_mouse_position);
  else
    mouse_motion = fvec2_zero();
}

void window_present()
{
  glfwSwapBuffers(window);
}

void window_show_cursor(bool cursor)
{
  glfwSetInputMode(window, GLFW_CURSOR, cursor ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

