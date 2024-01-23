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
  ui->program_quad         = 0;
  ui->program_quad_rounded = 0;
  ui->program_texture_mono = 0;
  ui->vao                  = 0;

  if((ui->program_quad         = gl_program_load("assets/ui_quad.vert",         "assets/ui_quad.frag"))         == 0) goto error;
  if((ui->program_quad_rounded = gl_program_load("assets/ui_quad_rounded.vert", "assets/ui_quad_rounded.frag")) == 0) goto error;
  if((ui->program_texture_mono = gl_program_load("assets/ui_texture_mono.vert", "assets/ui_texture_mono.frag")) == 0) goto error;

  glGenVertexArrays(1, &ui->vao);
  glBindVertexArray(ui->vao);
  return 0;

error:
  ui_deinit(ui);
  return -1;
}

void ui_deinit(struct ui *ui)
{
  if(ui->vao)                  glDeleteVertexArrays(1, &ui->vao);
  if(ui->program_texture_mono) glDeleteProgram(ui->program_texture_mono);
  if(ui->program_quad_rounded) glDeleteProgram(ui->program_quad_rounded);
  if(ui->program_quad)         glDeleteProgram(ui->program_quad);
}

void ui_draw_quad(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, struct vec4 color)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_quad);

  glUniform2f(glGetUniformLocation(ui->program_quad, "window_size"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_quad, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_quad, "dimension"  ), dimension  .x, dimension  .y);

  glUniform4f(glGetUniformLocation(ui->program_quad, "color"), color.r, color.g, color.b, color.a);

  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ui_draw_quad_rounded(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, float radius, struct vec4 color)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_quad_rounded);

  glUniform2f(glGetUniformLocation(ui->program_quad_rounded, "window_size"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_quad_rounded, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_quad_rounded, "dimension"  ), dimension  .x, dimension  .y);

  glUniform4f(glGetUniformLocation(ui->program_quad_rounded, "color"), color.r, color.g, color.b, color.a);
  glUniform1f(glGetUniformLocation(ui->program_quad_rounded, "radius"), radius);

  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ui_draw_texture_mono(struct ui *ui, struct vec2 window_size, struct vec2 position, struct vec2 dimension, GLuint texture)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_texture_mono);

  glUniform2f(glGetUniformLocation(ui->program_texture_mono, "window_size"), window_size.x, window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_texture_mono, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_texture_mono, "dimension"  ), dimension  .x, dimension  .y);

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
    ui_draw_texture_mono(ui, window_size, current_position, current_dimension, glyph->texture);
    position.x += glyph->advance;
  }
}
