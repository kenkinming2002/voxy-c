#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <libnet/client.h>
#include <stdint.h>

struct input_manager
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

int input_manager_init(struct input_manager *input_manager);
void input_manager_fini(struct input_manager *input_manager);

void input_manager_update(struct input_manager *input_manager, libnet_client_t client);

#endif // INPUT_MANAGER_H
