#ifndef VOXY_UI_H
#define VOXY_UI_H

#include "glad/glad.h"
#include "lin.h"
#include "font_set.h"

struct ui
{
  GLuint program_quad;
  GLuint program_quad_rounded;
  GLuint program_texture_mono;
  GLuint vao;

  struct vec2 window_size;
};

int ui_init(struct ui *ui);
void ui_deinit(struct ui *ui);

void ui_begin(struct ui *ui, struct vec2 window_size);

void ui_draw_quad(struct ui *ui, struct vec2 position, struct vec2 dimension, struct vec4 color);
void ui_draw_quad_rounded(struct ui *ui, struct vec2 position, struct vec2 dimension, float radius, struct vec4 color);
void ui_draw_texture_mono(struct ui *ui, struct vec2 position, struct vec2 dimension, GLuint texture);

void ui_draw_text(struct ui *ui, struct font_set *font_set, struct vec2 position, const char *str, unsigned height);
void ui_draw_text_centered(struct ui *ui, struct font_set *font_set, struct vec2 position, const char *st, unsigned heightr);

#endif // VOXY_UI_H
