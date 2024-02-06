#ifndef VOXY_WINDOW_H
#define VOXY_WINDOW_H

#include <voxy/math/vector.h>

#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

enum input
{
  KEY_1,
  KEY_2,
  KEY_3,
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,

  KEY_KP_1,
  KEY_KP_2,
  KEY_KP_3,
  KEY_KP_4,
  KEY_KP_5,
  KEY_KP_6,
  KEY_KP_7,
  KEY_KP_8,
  KEY_KP_9,

  KEY_A,
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G,
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q,
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,

  KEY_SPACE,
  KEY_SHIFT,
  KEY_CTRL,

  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_MIDDLE,

  INPUT_MAX,
};

_Static_assert(INPUT_MAX < SIZE_WIDTH, "Too many input");

struct window
{
  GLFWwindow *window;

  int width;
  int height;

  int framebuffer_width;
  int framebuffer_height;

  size_t states;
  size_t presses;
  size_t releases;

  fvec2_t mouse_position;
  fvec2_t mouse_motion;
  ivec2_t mouse_scroll;
};

int window_init(struct window *window, const char *title, unsigned width, unsigned height);
void window_fini(struct window *window);

int window_should_close(struct window *window);
void window_begin(struct window *window);
void window_end(struct window *window);

void window_set_cursor(struct window *window, int cursor);

#endif // VOXY_WINDOW_H
