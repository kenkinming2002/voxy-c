#ifndef VOXY_GRAPHICS_UI_H
#define VOXY_GRAPHICS_UI_H

#include <voxy/math/vector.h>

#include <graphics/gl.h>
#include <graphics/font_set.h>

void ui_draw_quad(fvec2_t position, fvec2_t dimension, fvec4_t color);
void ui_draw_quad_rounded(fvec2_t position, fvec2_t dimension, float radius, fvec4_t color);

void ui_draw_texture(fvec2_t position, fvec2_t dimension, GLuint texture);
void ui_draw_texture_mono(fvec2_t position, fvec2_t dimension, GLuint texture);

void ui_draw_text(struct font_set *font_set, fvec2_t position, const char *str, unsigned height);
void ui_draw_text_centered(struct font_set *font_set, fvec2_t position, const char *str, unsigned heightr);

#endif // VOXY_GRAPHICS_UI_H
