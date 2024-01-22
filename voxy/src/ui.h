#ifndef VOXY_UI_H
#define VOXY_UI_H

#include "glad/glad.h"
#include "lin.h"

struct ui
{
  GLuint program;
  GLuint vao;
};

int ui_init(struct ui *ui);
void ui_deinit(struct ui *ui);
void ui_draw(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, GLuint texture);

#endif // VOXY_UI_H
