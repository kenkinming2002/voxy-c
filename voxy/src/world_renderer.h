#ifndef VOXY_WORLD_RENDERER_H
#define VOXY_WORLD_RENDERER_H

#include "camera.h"
#include "glad/glad.h"
#include "resource_pack.h"
#include "world.h"

#include <stddef.h>

struct world_renderer
{
  GLuint chunk_program;
  GLuint chunk_block_texture_array;
};

int world_renderer_init(struct world_renderer *world_renderer, struct resource_pack *resource_pack);
void world_renderer_fini(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct resource_pack *resource_pack, struct world *world);
void world_renderer_render(struct world_renderer *world_renderer, struct world *world, struct camera *camera);

#endif // VOXY_WORLD_RENDERER_H
