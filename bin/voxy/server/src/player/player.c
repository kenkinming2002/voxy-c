#include "player.h"

#include <voxy/protocol/server.h>

#include <libmath/matrix.h>
#include <libmath/matrix_transform.h>

#include <stdlib.h>

struct voxy_player *voxy_player_create(libnet_client_proxy_t client_proxy)
{
  struct voxy_player *player = malloc(sizeof *player);

  player->count = 1;
  player->weak_count = 0;

  player->client_proxy = client_proxy;

  player->name = NULL;

  player->key_left = 0;
  player->key_right = 0;
  player->key_back = 0;
  player->key_front = 0;
  player->key_bottom = 0;
  player->key_top = 0;

  player->mouse_motion = fvec2_zero();

  return player;
}

static void voxy_player_free(struct voxy_player *player)
{
  free(player->name);
  free(player);
}


struct voxy_player *voxy_player_upgrade(struct voxy_player *player)
{
  if(player->count != 0)
    return voxy_player_get(player);
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
  --player->count;
  if(player->count == 0 && player->weak_count == 0)
    voxy_player_free(player);
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
    voxy_player_free(player);
}

void voxy_player_set_camera_follow_entity(struct voxy_player *player, entity_handle_t handle)
{
  struct voxy_server_camera_follow_entity_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY;
  message.handle = handle;
  libnet_server_send_message(player->client_proxy, &message.message.message);
}

fvec3_t voxy_player_get_movement_input(struct voxy_player *player)
{
  fvec3_t input = fvec3_zero();
  if(player->key_left)   input.x -= 1.0f;
  if(player->key_right)  input.x += 1.0f;
  if(player->key_back)   input.y -= 1.0f;
  if(player->key_front)  input.y += 1.0f;
  if(player->key_bottom) input.z -= 1.0f;
  if(player->key_top)    input.z += 1.0f;
  return input;
}

fvec2_t voxy_player_get_pan_input(struct voxy_player *player)
{
  fvec2_t input = player->mouse_motion;
  player->mouse_motion = fvec2_zero();
  return input;
}

bool voxy_player_get_left_mouse_button_input(struct voxy_player *player)
{
  return player->mouse_button_left;
}

/// Get right mouse button input of player.
bool voxy_player_get_right_mouse_button_input(struct voxy_player *player)
{
  return player->mouse_button_right;
}
