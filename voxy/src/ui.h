#ifndef VOXY_UI_H
#define VOXY_UI_H

#include "glad/glad.h"
#include "lin.h"
#include "font.h"

struct ui
{
  GLuint program_quad;
  GLuint program_texture_mono;
  GLuint vao;
};

int ui_init(struct ui *ui);
void ui_deinit(struct ui *ui);

void ui_draw_quad(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, struct vec4 color);
void ui_draw_texture_mono(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, GLuint texture);
void ui_render_text(struct ui *ui, struct font *font, struct vec2 window_size, struct vec2 position, const char *str);

#endif // VOXY_UI_H
