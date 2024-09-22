#include "manager.h"

#include <voxy/protocol/client.h>

#include <libcommon/core/window.h>

#include <stdbool.h>

int input_manager_init(struct input_manager *input_manager)
{
  input_manager->key_left = 0;
  input_manager->key_right = 0;
  input_manager->key_back = 0;
  input_manager->key_front = 0;
  input_manager->key_bottom = 0;
  input_manager->key_top = 0;
  return 0;
}

void input_manager_fini(struct input_manager *input_manager)
{
  (void)input_manager;
}

void input_manager_update(struct input_manager *input_manager, libnet_client_t client)
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

  if(input_manager->key_left != key_left)
  {
    input_manager->key_left = key_left;
    changed = true;
  }

  if(input_manager->key_right != key_right)
  {
    input_manager->key_right = key_right;
    changed = true;
  }

  if(input_manager->key_back != key_back)
  {
    input_manager->key_back = key_back;
    changed = true;
  }

  if(input_manager->key_front != key_front)
  {
    input_manager->key_front = key_front;
    changed = true;
  }

  if(input_manager->key_bottom != key_bottom)
  {
    input_manager->key_bottom = key_bottom;
    changed = true;
  }

  if(input_manager->key_top != key_top)
  {
    input_manager->key_top = key_top;
    changed = true;
  }

  if(input_manager->mouse_button_left != mouse_button_left)
  {
    input_manager->mouse_button_left = mouse_button_left;
    changed = true;
  }

  if(input_manager->mouse_button_right != mouse_button_right)
  {
    input_manager->mouse_button_right = mouse_button_right;
    changed = true;
  }

  if(fvec2_length_squared(mouse_position) != 0.0f)
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

    libnet_client_send_message(client, &message.message.message);
  }
}

