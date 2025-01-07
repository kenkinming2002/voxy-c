#ifndef LIBGFX_WINDOW_H
#define LIBGFX_WINDOW_H

#include <libmath/vector.h>

#include <stddef.h>
#include <stdbool.h>

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

  KEY_ESC,

  KEY_SPACE,
  KEY_SHIFT,
  KEY_CTRL,

  BUTTON_LEFT,
  BUTTON_RIGHT,
  BUTTON_MIDDLE,

  INPUT_MAX,
};
_Static_assert(INPUT_MAX < SIZE_WIDTH, "Too many input");

extern ivec2_t window_size;
extern ivec2_t framebuffer_size;

extern size_t input_states;
extern size_t input_presses;
extern size_t input_releases;

extern fvec2_t mouse_position;
extern fvec2_t mouse_motion;
extern ivec2_t mouse_scroll;

static inline bool input_state  (enum input input) { return input_states   & (1ULL << input); }
static inline bool input_press  (enum input input) { return input_presses  & (1ULL << input); }
static inline bool input_release(enum input input) { return input_releases & (1ULL << input); }

void window_init(const char *title, unsigned width, unsigned height);

bool window_should_close();

void window_update();
void window_present();

void window_show_cursor(bool cursor);

#endif // LIBGFX_WINDOW_H
