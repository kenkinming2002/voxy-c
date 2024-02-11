#ifndef WORLD_RENDER_H
#define WORLD_RENDER_H

struct world;
struct mod;
struct mod_assets;

void world_render(struct world *world, struct mod *mod, struct mod_assets *mod_assets);

#endif // WORLD_RENDER_H
