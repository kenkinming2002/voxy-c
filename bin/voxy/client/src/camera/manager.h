#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include "entity/manager.h"

#include <libcommon/graphics/camera.h>

#include <libnet/client.h>

struct camera_manager
{
  struct camera camera;
  entity_handle_t target;
};

int camera_manager_init(struct camera_manager *camera_manager);
void camera_manager_fini(struct camera_manager *camera_manager);

void camera_manager_update(struct camera_manager *camera_manager, struct entity_manager *entity_manager);
void camera_manager_on_message_received(struct camera_manager *camera_manager, libnet_client_t client, const struct libnet_message *_message);

#endif // CAMERA_MANAGER_H
