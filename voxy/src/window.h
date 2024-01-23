#ifndef VOXY_WINDOW_H
#define VOXY_WINDOW_H

#include "glad/glad.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

struct window
{
  GLFWwindow *window;
  double xpos, ypos;
};

int window_init(struct window *window, const char *title, unsigned width, unsigned height);
void window_deinit(struct window *window);

int window_should_close(struct window *window);
void window_swap_buffers(struct window *window);
void window_handle_events(struct window *window);

void window_get_framebuffer_size(struct window *window, int *width, int *height);
void window_get_mouse_motion(struct window *window, float *dx, float *dy);
void window_get_keyboard_motion(struct window *window, float *dx, float *dy, float *dz);

int window_get_key(struct window *window, int key);

#endif // VOXY_WINDOW_H
