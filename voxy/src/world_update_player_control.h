#ifndef VOXY_WORLD_UPDATE_PLAYER_CONTROL_H
#define VOXY_WORLD_UPDATE_PLAYER_CONTROL_H

struct world;
struct mod;

void world_update_player_control(struct world *world, struct mod *mod, float dt);

#endif // VOXY_WORLD_UPDATE_PLAYER_CONTROL_H
