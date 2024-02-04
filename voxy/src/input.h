#ifndef VOXY_INPUT_H
#define VOXY_INPUT_H

#include <voxy/math/vector.h>

#include <stdbool.h>

struct input
{
  fvec2_t mouse_position;
  fvec2_t mouse_motion;

  fvec3_t keyboard_motion;

  bool selects[9];
  int click_left;
  int click_right;
  int click_i;
  int scroll;

  bool state_left;
  bool state_right;
  bool state_ctrl;
};

#endif // VOXY_INPUT_H
