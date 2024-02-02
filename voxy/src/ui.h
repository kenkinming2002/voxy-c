#ifndef VOXY_UI_H
#define VOXY_UI_H

#include <voxy/math/vector.h>

#include "gl.h"
#include "font_set.h"

struct ui
{
  struct gl_program program_quad;
  struct gl_program program_quad_rounded;
  struct gl_program program_texture_mono;

  GLuint vao;

  fvec2_t window_size;
};

int ui_init(struct ui *ui);
void ui_fini(struct ui *ui);

void ui_begin(struct ui *ui, fvec2_t window_size);

void ui_draw_quad(struct ui *ui, fvec2_t position, fvec2_t dimension, fvec4_t color);
void ui_draw_quad_rounded(struct ui *ui, fvec2_t position, fvec2_t dimension, float radius, fvec4_t color);
void ui_draw_texture_mono(struct ui *ui, fvec2_t position, fvec2_t dimension, GLuint texture);

void ui_draw_text(struct ui *ui, struct font_set *font_set, fvec2_t position, const char *str, unsigned height);
void ui_draw_text_centered(struct ui *ui, struct font_set *font_set, fvec2_t position, const char *st, unsigned heightr);

#endif // VOXY_UI_H
