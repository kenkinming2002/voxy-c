#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <libnet/server.h>

#include <libcommon/math/vector.h>

#include <stdint.h>

/// Player.
///
/// This is stored internally in opaque pointer of client proxy.
struct player
{
  fvec3_t position;
  fvec3_t rotation;

  uint8_t left : 1;
  uint8_t right : 1;
  uint8_t back : 1;
  uint8_t front : 1;
  uint8_t bottom : 1;
  uint8_t top : 1;

  fvec2_t mouse_motion;
};

/// Create/destroy player.
struct player *player_create(void);
void player_destroy(struct player *player);

/// Update player.
void player_update(struct player *player, libnet_server_t server, libnet_client_proxy_t client_proxy, float dt);

#endif // PLAYER_PLAYER_H
