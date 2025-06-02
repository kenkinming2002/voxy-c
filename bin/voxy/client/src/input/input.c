#include "input.h"

#include <voxy/protocol/client.h>

#include <libgfx/window.h>

#include <stdbool.h>

struct state
{
  /// Keys.
  uint8_t key_left : 1;
  uint8_t key_right : 1;
  uint8_t key_back : 1;
  uint8_t key_front : 1;
  uint8_t key_bottom : 1;
  uint8_t key_top : 1;

  /// Mouse buttons.
  uint8_t mouse_button_left : 1;
  uint8_t mouse_button_right : 1;
};

static struct state state;

void input_update(void)
{
  const uint8_t key_left = input_state(KEY_A);
  const uint8_t key_right = input_state(KEY_D);
  const uint8_t key_back = input_state(KEY_S);
  const uint8_t key_front = input_state(KEY_W);
  const uint8_t key_bottom = input_state(KEY_SHIFT);
  const uint8_t key_top = input_state(KEY_SPACE);

  const uint8_t mouse_button_left = input_state(BUTTON_LEFT);
  const uint8_t mouse_button_right = input_state(BUTTON_RIGHT);

  bool changed = false;

  if(state.key_left != key_left)
  {
    state.key_left = key_left;
    changed = true;
  }

  if(state.key_right != key_right)
  {
    state.key_right = key_right;
    changed = true;
  }

  if(state.key_back != key_back)
  {
    state.key_back = key_back;
    changed = true;
  }

  if(state.key_front != key_front)
  {
    state.key_front = key_front;
    changed = true;
  }

  if(state.key_bottom != key_bottom)
  {
    state.key_bottom = key_bottom;
    changed = true;
  }

  if(state.key_top != key_top)
  {
    state.key_top = key_top;
    changed = true;
  }

  if(state.mouse_button_left != mouse_button_left)
  {
    state.mouse_button_left = mouse_button_left;
    changed = true;
  }

  if(state.mouse_button_right != mouse_button_right)
  {
    state.mouse_button_right = mouse_button_right;
    changed = true;
  }

  if(fvec2_length_squared(mouse_motion) != 0.0f)
    changed = true;

  if(changed)
  {
    struct voxy_client_input_message message;

    message.message.tag = VOXY_CLIENT_MESSAGE_INPUT;
    message.message.message.size = LIBNET_MESSAGE_SIZE(message);

    message.key_left = key_left;
    message.key_right = key_right;
    message.key_back = key_back;
    message.key_front = key_front;
    message.key_bottom = key_bottom;
    message.key_top = key_top;

    message.mouse_button_left = mouse_button_left;
    message.mouse_button_right = mouse_button_right;

    message.mouse_motion = mouse_motion;

    libnet_client_send_message(&message.message.message);
  }
}

