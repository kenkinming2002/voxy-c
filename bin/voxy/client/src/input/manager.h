#ifndef INPUT_MANAGER_H
#define INPUT_MANAGER_H

#include <libnet/client.h>
#include <stdint.h>

struct input_manager
{
  uint8_t left : 1;
  uint8_t right : 1;
  uint8_t back : 1;
  uint8_t front : 1;
  uint8_t bottom : 1;
  uint8_t top : 1;
};

int input_manager_init(struct input_manager *input_manager);
void input_manager_fini(struct input_manager *input_manager);

void input_manager_update(struct input_manager *input_manager, libnet_client_t client);

#endif // INPUT_MANAGER_H
