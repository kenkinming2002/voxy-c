#ifndef VOXY_RENDERER_WORLD_H
#define VOXY_RENDERER_WORLD_H

#include "gl.h"

struct world;
struct mod;
struct mod_assets;

struct renderer_world
{
  struct gl_program program_chunk;
  struct gl_program program_outline;
};

int renderer_world_init(struct renderer_world *renderer_world);
void renderer_world_fini(struct renderer_world *renderer_world);

void renderer_world_render(struct renderer_world *renderer_world, struct world *world, struct mod *mod, struct mod_assets *mod_assets);

#endif // VOXY_RENDERER_WORLD_H
