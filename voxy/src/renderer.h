#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include "camera.h"
#include "font_set.h"
#include "glad/glad.h"
#include "resource_pack.h"
#include "ui.h"
#include "window.h"
#include "world.h"

struct renderer
{
  GLuint chunk_program;
  GLuint chunk_block_texture_array;

  struct ui       ui;
  struct font_set font_set;
};

int renderer_init(struct renderer *renderer, struct resource_pack *resource_pack);
void renderer_fini(struct renderer *renderer);

void renderer_render(struct renderer *renderer, struct window *window, struct camera *camera, struct world *world);
void renderer_render_chunks(struct renderer *renderer, struct camera *camera, struct world *world);
void renderer_render_ui(struct renderer *renderer, struct window *window, struct world *world);

#endif // VOXY_RENDERER_H
