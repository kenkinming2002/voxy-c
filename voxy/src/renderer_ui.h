#ifndef VOXY_RENDERER_UI_H
#define VOXY_RENDERER_UI_H

#include <voxy/math/vector.h>

#include "gl.h"

struct renderer_ui
{
  struct gl_program program_quad;
  struct gl_program program_quad_rounded;
  struct gl_program program_texture;
  struct gl_program program_texture_mono;

  GLuint vao;

  fvec2_t window_size;
};

struct font_set;

int renderer_ui_init(struct renderer_ui *renderer_ui);
void renderer_ui_fini(struct renderer_ui *renderer_ui);

void renderer_ui_begin(struct renderer_ui *renderer_ui, fvec2_t window_size);

void renderer_ui_draw_quad(struct renderer_ui *renderer_ui, fvec2_t position, fvec2_t dimension, fvec4_t color);
void renderer_ui_draw_quad_rounded(struct renderer_ui *renderer_ui, fvec2_t position, fvec2_t dimension, float radius, fvec4_t color);
void renderer_ui_draw_texture(struct renderer_ui *renderer_ui, fvec2_t position, fvec2_t dimension, GLuint texture);
void renderer_ui_draw_texture_mono(struct renderer_ui *renderer_ui, fvec2_t position, fvec2_t dimension, GLuint texture);

void renderer_ui_draw_text(struct renderer_ui *renderer_ui, struct font_set *font_set, fvec2_t position, const char *str, unsigned height);
void renderer_ui_draw_text_centered(struct renderer_ui *renderer_ui, struct font_set *font_set, fvec2_t position, const char *st, unsigned heightr);

#endif // VOXY_RENDERER_UI_H
