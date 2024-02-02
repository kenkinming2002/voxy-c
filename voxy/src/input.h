#ifndef VOXY_INPUT_H
#define VOXY_INPUT_H

#include <voxy/math/vector.h>

#include <stdbool.h>

struct input
{
  fvec2_t mouse_motion;
  fvec3_t keyboard_motion;

  bool selects[9];
  int scroll;
};

#endif // VOXY_INPUT_H
