#ifndef VOXY_SERVER_PLAYER_MANAGER_H
#define VOXY_SERVER_PLAYER_MANAGER_H

#include <voxy/server/export.h>
#include <voxy/server/chunk/entity/manager.h>

#include <libmath/vector.h>

struct voxy_context;
struct voxy_player;
struct voxy_player_manager;

typedef void(*voxy_on_new_player)(struct voxy_player *player, const struct voxy_context *context);

/// Set the callback function used to spawn a player entity.
VOXY_SERVER_EXPORT void voxy_player_manager_set_on_new_player(struct voxy_player_manager *player_manager, voxy_on_new_player on_new_player);

#endif // VOXY_SERVER_PLAYER_MANAGER_H
