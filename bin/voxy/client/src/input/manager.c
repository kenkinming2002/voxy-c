#include "manager.h"

#include <voxy/protocol/client.h>

#include <libcommon/core/window.h>

#include <stdbool.h>

int input_manager_init(struct input_manager *input_manager)
{
  input_manager->left = 0;
  input_manager->right = 0;
  input_manager->back = 0;
  input_manager->front = 0;
  input_manager->bottom = 0;
  input_manager->top = 0;
  return 0;
}

void input_manager_fini(struct input_manager *input_manager)
{
  (void)input_manager;
}

void input_manager_update(struct input_manager *input_manager, libnet_client_t client)
{
  const uint8_t left = input_state(KEY_A);
  const uint8_t right = input_state(KEY_D);
  const uint8_t back = input_state(KEY_S);
  const uint8_t front = input_state(KEY_W);
  const uint8_t bottom = input_state(KEY_SHIFT);
  const uint8_t top = input_state(KEY_SPACE);

  bool changed = false;

  if(input_manager->left != left)
  {
    input_manager->left = left;
    changed = true;
  }

  if(input_manager->right != right)
  {
    input_manager->right = right;
    changed = true;
  }

  if(input_manager->back != back)
  {
    input_manager->back = back;
    changed = true;
  }

  if(input_manager->front != front)
  {
    input_manager->front = front;
    changed = true;
  }

  if(input_manager->bottom != bottom)
  {
    input_manager->bottom = bottom;
    changed = true;
  }

  if(input_manager->top != top)
  {
    input_manager->top = top;
    changed = true;
  }

  if(fvec2_length_squared(mouse_position) != 0.0f)
    changed = true;

  if(changed)
  {
    struct voxy_client_input_message message;

    message.message.tag = VOXY_CLIENT_MESSAGE_INPUT;
    message.message.message.size = LIBNET_MESSAGE_SIZE(message);

    message.left = left;
    message.right = right;
    message.back = back;
    message.front = front;
    message.bottom = bottom;
    message.top = top;

    message.motion = mouse_motion;

    libnet_client_send_message(client, &message.message.message);
  }
}

