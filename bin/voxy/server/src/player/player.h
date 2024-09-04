#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <voxy/server/player/player.h>

#include "entity/manager.h"

#include <libnet/server.h>

#include <libcommon/math/vector.h>

#include <stdint.h>

/// Player.
///
/// This is stored internally in opaque pointer of client proxy.
struct voxy_player
{
  entity_handle_t handle;

  uint8_t left : 1;
  uint8_t right : 1;
  uint8_t back : 1;
  uint8_t front : 1;
  uint8_t bottom : 1;
  uint8_t top : 1;

  fvec2_t mouse_motion;
};

/// Create/destroy player.
struct voxy_player *player_create(struct voxy_entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);
void player_destroy(struct voxy_player *player, struct voxy_entity_manager *entity_manager, libnet_server_t server);

/// Update player.
void player_update(struct voxy_player *player, float dt, struct voxy_entity_manager *entity_manager);

#endif // PLAYER_PLAYER_H
