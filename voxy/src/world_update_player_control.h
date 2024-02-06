#ifndef VOXY_WORLD_UPDATE_PLAYER_CONTROL_H
#define VOXY_WORLD_UPDATE_PLAYER_CONTROL_H

struct world;
struct resource_pack;
struct window;

void world_update_player_control(struct world *world, struct resource_pack *resource_pack, struct window *window, float dt);

#endif // VOXY_WORLD_UPDATE_PLAYER_CONTROL_H
