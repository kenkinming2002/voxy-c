#include "player.h"

#include <voxy/protocol/server.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

#include <stdlib.h>

struct voxy_player *voxy_player_create(libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = malloc(sizeof *player);

  player->count = 1;
  player->weak_count = 0;

  player->server = server;
  player->client_proxy = client_proxy;

  player->left = 0;
  player->right = 0;
  player->back = 0;
  player->front = 0;
  player->bottom = 0;
  player->top = 0;

  player->mouse_motion = fvec2_zero();

  return player;
}

struct voxy_player *voxy_player_upgrade(struct voxy_player *player)
{
  if(player->count != 0)
  {
    voxy_player_get(player);
    return player;
  }
  else
    return NULL;
}

struct voxy_player *voxy_player_get(struct voxy_player *player)
{
  ++player->count;
  return player;
}

void voxy_player_put(struct voxy_player *player)
{
  --player->weak_count;
  if(player->count == 0 && player->weak_count == 0)
    free(player);
}

struct voxy_player *voxy_player_get_weak(struct voxy_player *player)
{
  ++player->weak_count;
  return player;
}

void voxy_player_put_weak(struct voxy_player *player)
{
  --player->weak_count;
  if(player->count == 0 && player->weak_count == 0)
    free(player);
}

void voxy_player_set_camera_follow_entity(struct voxy_player *player, entity_handle_t handle)
{
  struct voxy_server_camera_follow_entity_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY;
  message.handle = handle;
  libnet_server_send_message(player->server, player->client_proxy, &message.message.message);
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

