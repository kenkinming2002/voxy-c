#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include "camera.h"
#include "font_set.h"
#include "glad/glad.h"
#include "ui.h"
#include "window.h"
#include "world.h"
#include "gl.h"

struct renderer
{
  struct gl_program chunk_program;
  struct ui         ui;
};

int renderer_init(struct renderer *renderer);
void renderer_fini(struct renderer *renderer);

void renderer_render(struct renderer *renderer, int width, int height, struct resource_pack *resource_pack, struct camera *camera, struct world *world);
void renderer_render_chunks(struct renderer *renderer, struct camera *camera, struct resource_pack *resource_pack, struct world *world);
void renderer_render_ui(struct renderer *renderer, int width, int height, struct resource_pack *resource_pack, struct world *world);

#endif // VOXY_RENDERER_H
