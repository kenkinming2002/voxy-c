#include "ui.h"

#include "gl.h"
#include "lin.h"

#include <assert.h>

struct ui_vertex
{
  struct vec2 position;
  struct vec2 texture_coords;
};

int ui_init(struct ui *ui)
{
  if((ui->program = gl_program_load("assets/ui.vert", "assets/ui.frag")) == 0)
    goto error1;

  glGenVertexArrays(1, &ui->vao);
  glBindVertexArray(ui->vao);
  return 0;

error1:
  return -1;
}

void ui_deinit(struct ui *ui)
{
  glDeleteVertexArrays(1, &ui->vao);
}

void ui_draw(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, GLuint texture)
{
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program);

  glUniform2f(glGetUniformLocation(ui->program, "window_size"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(ui->program, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program, "dimension"  ), dimension  .x, dimension  .y);

  glBindTexture(GL_TEXTURE_2D, texture);
  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

static int utf8_next(const unsigned char **str)
{
  int c1 = *(*str)++;
  if((c1 >> 7) == 0b0)
    return c1;

  if((c1 >> 5) == 0b110)
  {
    int c2 = *(*str)++;
    assert(c2 >> 6 == 0b10 && "Expected unicode continuation byte");
    return (c1 & 0b11111)  << 6
         | (c2 & 0b111111) << 0;
  }

  if((c1 >> 4) == 0b1110)
  {
    int c2 = *(*str)++;
    int c3 = *(*str)++;
    assert(c2 >> 6 == 0b10 && "Expected unicode continuation byte");
    assert(c3 >> 6 == 0b10 && "Expected unicode continuation byte");
    return (c1 & 0b11111)  << 12
         | (c2 & 0b111111) << 6
         | (c3 & 0b111111) << 0;
  }

  if((c1 >> 3) == 0b11110)
  {
    int c2 = *(*str)++;
    int c3 = *(*str)++;
    int c4 = *(*str)++;
    assert(c2 >> 6 == 0b10 && "Expected unicode continuation byte");
    assert(c3 >> 6 == 0b10 && "Expected unicode continuation byte");
    assert(c4 >> 6 == 0b10 && "Expected unicode continuation byte");
    return (c1 & 0b11111)  << 18
         | (c2 & 0b111111) << 12
         | (c3 & 0b111111) << 6
         | (c4 & 0b111111) << 0;
  }

  assert(0 && "Invalid UTF-8 String");
}

void ui_render_text(struct ui *ui, struct font *font, struct vec2 window_size, struct vec2 position, const char *str)
{
  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_get_glyph(font, c);
    struct vec2 current_position  = vec2_add(position, glyph->bearing);
    struct vec2 current_dimension = vec2_mul(glyph->dimension, vec2(1.0f, -1.0f));
    ui_draw(ui, window_size, current_position, current_dimension, glyph->texture);
    position.x += glyph->advance;
  }
}
