#ifndef VOXY_WINDOW_H
#define VOXY_WINDOW_H

#include "input.h"
#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct window
{
  GLFWwindow *window;

  bool cursor;

  fvec2_t mouse_position;

  int click_left;
  int click_right;
  int click_i;
  int scroll;
};

int window_init(struct window *window, const char *title, unsigned width, unsigned height);
void window_fini(struct window *window);

int window_should_close(struct window *window);
void window_swap_buffers(struct window *window);
void window_handle_events(struct window *window);

void window_get_framebuffer_size(struct window *window, int *width, int *height);

void window_get_cursor(struct window *window, bool *cursor);
void window_set_cursor(struct window *window, bool cursor);

void window_get_input(struct window *window, struct input *input);

fvec2_t window_get_mouse_position(struct window *window);
fvec2_t window_get_mouse_motion(struct window *window);
fvec3_t window_get_keyboard_motion(struct window *window);

int window_get_key(struct window *window, int key);

#endif // VOXY_WINDOW_H
