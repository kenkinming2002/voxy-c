#ifndef WORLD_UPDATE_H
#define WORLD_UPDATE_H

struct world;
struct world_generator;
struct mod;
struct mod_assets;

void world_update(struct world *world, struct world_generator *world_generator, struct mod *mod, float dt);

#endif // WORLD_UPDATE_H
