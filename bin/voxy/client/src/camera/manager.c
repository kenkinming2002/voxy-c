#include "manager.h"

#include <voxy/protocol/server.h>

#include <libcommon/core/window.h>
#include <libcommon/core/log.h>

int camera_manager_init(struct camera_manager *camera_manager)
{
  camera_manager->camera.fovy = M_PI / 2.0f;
  camera_manager->camera.near = 0.1f;
  camera_manager->camera.far = 1000.0f;
  camera_manager->camera.aspect = (float)window_size.x / (float)window_size.y;
  camera_manager->camera.transform.translation = fvec3_zero();
  camera_manager->camera.transform.rotation = fvec3_zero();
  return 0;
}

void camera_manager_fini(struct camera_manager *camera_manager)
{
  (void)camera_manager;
}

void camera_manager_update(struct camera_manager *camera_manager)
{
  camera_manager->camera.aspect = (float)window_size.x / (float)window_size.y;
}

void camera_manager_on_message_received(struct camera_manager *camera_manager, libnet_client_t client, const struct libnet_message *_message)
{
  struct voxy_server_camera_update_message *message = voxy_get_server_camera_update_message(_message);
  if(message)
  {
    camera_manager->camera.transform.translation = message->position;
    camera_manager->camera.transform.rotation = message->rotation;
  }
}
