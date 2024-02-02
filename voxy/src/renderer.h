#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include "camera.h"
#include "glad/glad.h"
#include "resource_pack.h"
#include "window.h"
#include "world.h"

struct renderer
{
  GLuint chunk_program;
  GLuint chunk_block_texture_array;
};

int renderer_init(struct renderer *renderer, struct resource_pack *resource_pack);
void renderer_fini(struct renderer *renderer);

void renderer_render(struct renderer *renderer, struct window *window, struct camera *camera, struct world *world);
void renderer_render_chunks(struct renderer *renderer, struct camera *camera, struct world *world);

#endif // VOXY_RENDERER_H
