#ifndef VOXY_SERVER_PLAYER_PLAYER_H
#define VOXY_SERVER_PLAYER_PLAYER_H

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>

/// Player type.
///
/// Each instance corresponds to a single client/connection.
struct voxy_player;

/// Get movement input of player.
///
/// This supposedly corresponds to WASD, shift and space keys on the client, but
/// it is up to client to rebind any key as it sees fit.
VOXY_SERVER_EXPORT fvec3_t voxy_player_get_movement_input(struct voxy_player *player);

/// Get pan input of player.
///
/// This supposedly corresponds to mouse motion on the client, but it is up to
/// client to rebind any key as it sees fit.
///
/// Input are accumulated internaly, and reset to zero on call to this function.
VOXY_SERVER_EXPORT fvec2_t voxy_player_get_pan_input(struct voxy_player *player);

#endif // VOXY_SERVER_PLAYER_PLAYER_H
