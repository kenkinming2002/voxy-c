#include "manager.h"

#include <voxy/protocol/server.h>

#include <libgfx/window.h>
#include <libcore/log.h>

int camera_manager_init(struct camera_manager *camera_manager)
{
  camera_manager->camera.fovy = M_PI / 2.0f;
  camera_manager->camera.near = 0.1f;
  camera_manager->camera.far = 1000.0f;
  camera_manager->camera.aspect = (float)window_size.x / (float)window_size.y;
  camera_manager->camera.transform.translation = fvec3_zero();
  camera_manager->camera.transform.rotation = fvec3_zero();
  camera_manager->target = ENTITY_HANDLE_NULL;
  return 0;
}

void camera_manager_fini(struct camera_manager *camera_manager)
{
  (void)camera_manager;
}

void camera_manager_update(struct camera_manager *camera_manager, struct entity_manager *entity_manager)
{
  camera_manager->camera.aspect = (float)window_size.x / (float)window_size.y;
  if(camera_manager->target != ENTITY_HANDLE_NULL)
  {
    const struct voxy_entity *entity = entity_manager_get(entity_manager, camera_manager->target);
    camera_manager->camera.transform.translation = entity->position;
    camera_manager->camera.transform.rotation = entity->rotation;
  }
}

void camera_manager_on_message_received(struct camera_manager *camera_manager, libnet_client_t client, const struct libnet_message *_message)
{
  struct voxy_server_camera_follow_entity_message *message = voxy_get_server_camera_follow_entity_message(_message);
  if(message)
    camera_manager->target = message->handle;
}
