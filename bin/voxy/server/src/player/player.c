#include "player.h"

#include <voxy/protocol/server.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

#include <stdlib.h>

struct voxy_player *player_create(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = malloc(sizeof *player);
  player->handle = voxy_entity_manager_spawn(entity_manager, 0, fvec3_zero(), fvec3_zero(), NULL, server);
  player->left = 0;
  player->right = 0;
  player->back = 0;
  player->front = 0;
  player->bottom = 0;
  player->top = 0;
  player->mouse_motion = fvec2_zero();

  struct voxy_server_camera_follow_entity_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY;
  message.handle = player->handle;
  libnet_server_send_message(server, client_proxy, &message.message.message);

  return player;
}

void player_destroy(struct voxy_player *player, struct voxy_entity_manager *entity_manager, libnet_server_t server)
{
  voxy_entity_manager_despawn(entity_manager, player->handle, server);
  free(player);
}

fvec3_t voxy_player_get_movement_input(struct voxy_player *player)
{
  fvec3_t input = fvec3_zero();
  if(player->left)   input.x -= 1.0f;
  if(player->right)  input.x += 1.0f;
  if(player->back)   input.y -= 1.0f;
  if(player->front)  input.y += 1.0f;
  if(player->bottom) input.z -= 1.0f;
  if(player->top)    input.z += 1.0f;
  return input;
}

fvec2_t voxy_player_get_pan_input(struct voxy_player *player)
{
  fvec2_t input = player->mouse_motion;
  player->mouse_motion = fvec2_zero();
  return input;
}

void player_update(struct voxy_player *player, float dt, struct voxy_entity_manager *entity_manager)
{
  struct voxy_entity *entity = voxy_entity_manager_get(entity_manager, player->handle);

  const fvec3_t movement = voxy_player_get_movement_input(player);
  {
    fvec4_t offset4 = fmat4_mul_vec(fmat4_rotate(entity->rotation), fvec4(movement.x, movement.y, movement.z, 1.0f));
    fvec3_t offset = fvec3(offset4.x, offset4.y, offset4.z);
    offset = fvec3_mul_scalar(offset, dt);
    offset = fvec3_mul_scalar(offset, 100.0f);
    entity->position = fvec3_add(entity->position, offset);
  }

  const fvec2_t pan = voxy_player_get_pan_input(player);
  {
    entity->rotation.yaw   +=  pan.x * 0.002f;
    entity->rotation.pitch += -pan.y * 0.002f;
  }
}
