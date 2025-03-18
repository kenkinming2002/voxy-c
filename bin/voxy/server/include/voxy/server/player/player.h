#ifndef VOXY_SERVER_PLAYER_PLAYER_H
#define VOXY_SERVER_PLAYER_PLAYER_H

#include <voxy/server/export.h>
#include <voxy/server/chunk/entity/manager.h>

#include <libmath/vector.h>

#include <stdbool.h>

/// Player type.
///
/// Each instance corresponds to a single client/connection.
struct voxy_player;

/// Upgrade a weak reference to player to a strong reference to player.
VOXY_SERVER_EXPORT struct voxy_player *voxy_player_upgrade(struct voxy_player *player);

/// Get/put a strong reference to player.
VOXY_SERVER_EXPORT struct voxy_player *voxy_player_get(struct voxy_player *player);
VOXY_SERVER_EXPORT void voxy_player_put(struct voxy_player *player);

/// Get/put a weak reference to player.
VOXY_SERVER_EXPORT struct voxy_player *voxy_player_get_weak(struct voxy_player *player);
VOXY_SERVER_EXPORT void voxy_player_put_weak(struct voxy_player *player);

/// Set the entity that the player camera should follow.
VOXY_SERVER_EXPORT void voxy_player_set_camera_follow_entity(struct voxy_player *player, entity_handle_t handle);

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

/// Get left mouse button input of player.
VOXY_SERVER_EXPORT bool voxy_player_get_left_mouse_button_input(struct voxy_player *player);

/// Get right mouse button input of player.
VOXY_SERVER_EXPORT bool voxy_player_get_right_mouse_button_input(struct voxy_player *player);

#endif // VOXY_SERVER_PLAYER_PLAYER_H
