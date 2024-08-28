#include "player.h"

#include <voxy/protocol/server.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

#include <stdlib.h>

struct player *player_create(void)
{
  struct player *player = malloc(sizeof *player);

  player->position = fvec3_zero();
  player->rotation = fvec3_zero();

  player->left = 0;
  player->right = 0;
  player->back = 0;
  player->front = 0;
  player->bottom = 0;
  player->top = 0;

  player->mouse_motion = fvec2_zero();

  return player;
}

void player_destroy(struct player *player)
{
  free(player);
}

static void player_update_movement(struct player *player, float dt)
{
  fvec3_t axis = fvec3_zero();
  if(player->left)   axis.x -= 1.0f;
  if(player->right)  axis.x += 1.0f;
  if(player->back)   axis.y -= 1.0f;
  if(player->front)  axis.y += 1.0f;
  if(player->bottom) axis.z -= 1.0f;
  if(player->top)    axis.z += 1.0f;

  fvec4_t offset4 = fmat4_mul_vec(fmat4_rotate(player->rotation), fvec4(axis.x, axis.y, axis.z, 1.0f));
  fvec3_t offset = fvec3(offset4.x, offset4.y, offset4.z);
  offset = fvec3_mul_scalar(offset, dt);
  offset = fvec3_mul_scalar(offset, 5.0f);
  player->position = fvec3_add(player->position, offset);
}

static void player_update_rotation(struct player *player)
{
  player->rotation.yaw   +=  player->mouse_motion.x * 0.002f;
  player->rotation.pitch += -player->mouse_motion.y * 0.002f;
  player->mouse_motion = fvec2_zero();
}

static void player_update_network(struct player *player, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_server_camera_update_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CAMREA_UPDATE;
  message.position = player->position;
  message.rotation = player->rotation;
  libnet_server_send_message(server, client_proxy, &message.message.message);
}

void player_update(struct player *player, libnet_server_t server, libnet_client_proxy_t client_proxy, float dt)
{
  player_update_movement(player, dt);
  player_update_rotation(player);
  player_update_network(player, server, client_proxy);
}
