#ifndef VOXY_WORLD_UPDATE_PHYSICS_H
#define VOXY_WORLD_UPDATE_PHYSICS_H

struct world;
struct resource_pack;

void world_update_physics(struct world *world, struct resource_pack *resource_pack, float dt);

#endif // VOXY_WORLD_UPDATE_PHYSICS_H
