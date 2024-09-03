#include "player.h"

#include <voxy/protocol/server.h>

#include <libcommon/math/matrix.h>
#include <libcommon/math/matrix_transform.h>

#include <stdlib.h>

struct player *player_create(struct entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy)
{
  struct player *player = malloc(sizeof *player);
  player->handle = entity_manager_alloc(entity_manager);
  player->left = 0;
  player->right = 0;
  player->back = 0;
  player->front = 0;
  player->bottom = 0;
  player->top = 0;
  player->mouse_motion = fvec2_zero();

  struct entity *entity = entity_manager_get(entity_manager, player->handle);
  entity->id = 0;
  entity->position = fvec3_zero();
  entity->rotation = fvec3_zero();

  struct voxy_server_camera_follow_entity_message message;
  message.message.message.size = LIBNET_MESSAGE_SIZE(message);
  message.message.tag = VOXY_SERVER_MESSAGE_CAMERA_FOLLOW_ENTITY;
  message.handle = player->handle;
  libnet_server_send_message(server, client_proxy, &message.message.message);

  return player;
}

void player_destroy(struct player *player, struct entity_manager *entity_manager)
{
  entity_manager_free(entity_manager, player->handle);
  free(player);
}

static void player_update_movement(struct player *player, float dt, struct entity_manager *entity_manager)
{
  struct entity *entity = entity_manager_get(entity_manager, player->handle);

  fvec3_t axis = fvec3_zero();
  if(player->left)   axis.x -= 1.0f;
  if(player->right)  axis.x += 1.0f;
  if(player->back)   axis.y -= 1.0f;
  if(player->front)  axis.y += 1.0f;
  if(player->bottom) axis.z -= 1.0f;
  if(player->top)    axis.z += 1.0f;

  fvec4_t offset4 = fmat4_mul_vec(fmat4_rotate(entity->rotation), fvec4(axis.x, axis.y, axis.z, 1.0f));
  fvec3_t offset = fvec3(offset4.x, offset4.y, offset4.z);
  offset = fvec3_mul_scalar(offset, dt);
  offset = fvec3_mul_scalar(offset, 5.0f);
  entity->position = fvec3_add(entity->position, offset);
}

static void player_update_rotation(struct player *player, struct entity_manager *entity_manager)
{
  struct entity *entity = entity_manager_get(entity_manager, player->handle);
  entity->rotation.yaw   +=  player->mouse_motion.x * 0.002f;
  entity->rotation.pitch += -player->mouse_motion.y * 0.002f;
  player->mouse_motion = fvec2_zero();
}

void player_update(struct player *player, float dt, struct entity_manager *entity_manager)
{
  player_update_movement(player, dt, entity_manager);
  player_update_rotation(player, entity_manager);
}
