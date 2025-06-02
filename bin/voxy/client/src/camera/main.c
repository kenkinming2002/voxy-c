#include "main.h"

#include <voxy/protocol/server.h>

#include <libgfx/window.h>
#include <libcore/log.h>

static struct camera camera;
static entity_handle_t target;

void main_camera_init(void)
{
  camera.fovy = M_PI / 2.0f;
  camera.near = 0.1f;
  camera.far = 1000.0f;
  camera.aspect = (float)window_size.x / (float)window_size.y;
  camera.transform.translation = fvec3_zero();
  camera.transform.rotation = fvec3_zero();
  target = ENTITY_HANDLE_NULL;
}

void main_camera_update(void)
{
  camera.aspect = (float)window_size.x / (float)window_size.y;
  if(target != ENTITY_HANDLE_NULL)
  {
    const struct voxy_entity *entity = entity_get(target);
    camera.transform.translation = entity->position;
    camera.transform.rotation = entity->rotation;
  }
}

void main_camera_on_message_received(libnet_client_t client, const struct libnet_message *_message)
{
  struct voxy_server_camera_follow_entity_message *message = voxy_get_server_camera_follow_entity_message(_message);
  if(message)
    target = message->handle;
}

entity_handle_t get_main_camera_target(void)
{
  return target;
}

struct camera get_main_camera(void)
{
  return camera;
}
