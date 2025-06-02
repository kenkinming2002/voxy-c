#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "chunk/entity/manager.h"

#include <libgfx/camera.h>

#include <libnet/client.h>

void main_camera_init(void);
void main_camera_update(struct entity_manager *entity_manager);
void main_camera_on_message_received(libnet_client_t client, const struct libnet_message *_message);

struct camera get_main_camera(void);
entity_handle_t get_main_camera_target(void);

#endif // CAMERA_MANAGER_H
