#ifndef VOXY_WORLD_UPDATE_PHYSICS_H
#define VOXY_WORLD_UPDATE_PHYSICS_H

struct world;
struct mod;

void world_update_physics(struct world *world, struct mod *mod, float dt);

#endif // VOXY_WORLD_UPDATE_PHYSICS_H
