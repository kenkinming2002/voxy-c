#ifndef PLAYER_PLAYER_H
#define PLAYER_PLAYER_H

#include <voxy/server/player/player.h>

#include <libnet/server.h>

#include <libcommon/math/vector.h>

#include <stdint.h>

/// Player.
///
/// This is stored internally in opaque pointer of client proxy.
struct voxy_player
{
  unsigned count;
  unsigned weak_count;

  libnet_server_t server;
  libnet_client_proxy_t client_proxy;

  uint8_t left : 1;
  uint8_t right : 1;
  uint8_t back : 1;
  uint8_t front : 1;
  uint8_t bottom : 1;
  uint8_t top : 1;

  fvec2_t mouse_motion;
};

/// Create player.
struct voxy_player *voxy_player_create(libnet_server_t server, libnet_client_proxy_t client_proxy);

#endif // PLAYER_PLAYER_H
