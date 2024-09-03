#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include "entity/manager.h"

#include <libnet/server.h>

#include <libcommon/math/vector.h>

#include <stdint.h>

/// Player.
///
/// This is stored internally in opaque pointer of client proxy.
struct player
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
struct player *player_create(struct entity_manager *entity_manager);
void player_destroy(struct player *player, struct entity_manager *entity_manager);

/// Update player.
void player_update(struct player *player, float dt, struct entity_manager *entity_manager, libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // PLAYER_PLAYER_H
