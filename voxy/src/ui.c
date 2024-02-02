#include "ui.h"

#include "check.h"

#include <voxy/math/vector.h>

#include <assert.h>
#include <stdbool.h>

int ui_init(struct ui *ui)
{
  VOXY_CHECK_DECLARE(program_quad);
  VOXY_CHECK_DECLARE(program_quad_rounded);
  VOXY_CHECK_DECLARE(program_texture_mono);

  VOXY_CHECK_INIT(program_quad,         gl_program_load(&ui->program_quad,         2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/ui_quad.vert",         "assets/ui_quad.frag"        }));
  VOXY_CHECK_INIT(program_quad_rounded, gl_program_load(&ui->program_quad_rounded, 2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/ui_quad_rounded.vert", "assets/ui_quad_rounded.frag"}));
  VOXY_CHECK_INIT(program_texture_mono, gl_program_load(&ui->program_texture_mono, 2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/ui_texture_mono.vert", "assets/ui_texture_mono.frag"}));

  glGenVertexArrays(1, &ui->vao);
  glBindVertexArray(ui->vao);

  return 0;

error:
  VOXY_CHECK_FINI(program_quad,         gl_program_fini(&ui->program_quad));
  VOXY_CHECK_FINI(program_quad_rounded, gl_program_fini(&ui->program_quad_rounded));
  VOXY_CHECK_FINI(program_texture_mono, gl_program_fini(&ui->program_texture_mono));
  return -1;
}

void ui_fini(struct ui *ui)
{
  gl_program_fini(&ui->program_quad);
  gl_program_fini(&ui->program_quad_rounded);
  gl_program_fini(&ui->program_texture_mono);
}

void ui_begin(struct ui *ui, fvec2_t window_size)
{
  ui->window_size = window_size;
}

void ui_draw_quad(struct ui *ui, fvec2_t position, fvec2_t dimension, fvec4_t color)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_quad.id);

  glUniform2f(glGetUniformLocation(ui->program_quad.id, "window_size"), ui->window_size.x, ui->window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_quad.id, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_quad.id, "dimension"  ), dimension  .x, dimension  .y);

  glUniform4f(glGetUniformLocation(ui->program_quad.id, "color"), color.r, color.g, color.b, color.a);

  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ui_draw_quad_rounded(struct ui *ui, fvec2_t position, fvec2_t dimension, float radius, fvec4_t color)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_quad_rounded.id);

  glUniform2f(glGetUniformLocation(ui->program_quad_rounded.id, "window_size"), ui->window_size.x, ui->window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_quad_rounded.id, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_quad_rounded.id, "dimension"  ), dimension  .x, dimension  .y);

  glUniform4f(glGetUniformLocation(ui->program_quad_rounded.id, "color"), color.r, color.g, color.b, color.a);
  glUniform1f(glGetUniformLocation(ui->program_quad_rounded.id, "radius"), radius);

  glBindVertexArray(ui->vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ui_draw_texture_mono(struct ui *ui, fvec2_t position, fvec2_t dimension, GLuint texture)
{
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(ui->program_texture_mono.id);

  glUniform2f(glGetUniformLocation(ui->program_texture_mono.id, "window_size"), ui->window_size.x, ui->window_size.y);
  glUniform2f(glGetUniformLocation(ui->program_texture_mono.id, "position"   ), position   .x, position   .y);
  glUniform2f(glGetUniformLocation(ui->program_texture_mono.id, "dimension"  ), dimension  .x, dimension  .y);

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
    return (c1 & 0b1111)   << 12
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
    return (c1 & 0b111)    << 18
         | (c2 & 0b111111) << 12
         | (c3 & 0b111111) << 6
         | (c4 & 0b111111) << 0;
  }

  assert(0 && "Invalid UTF-8 String");
}

void ui_draw_text(struct ui *ui, struct font_set *font_set, fvec2_t position, const char *str, unsigned height)
{
  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(font_set, c, height);
    fvec2_t current_position  = fvec2_add(position, glyph->bearing);
    fvec2_t current_dimension = fvec2_mul(glyph->dimension, fvec2(1.0f, -1.0f));
    ui_draw_texture_mono(ui, current_position, current_dimension, glyph->texture);
    position.x += glyph->advance;
  }
}

static float font_compute_text_width(struct font_set *font_set, const char *str, unsigned height)
{
  float width = 0.0f;
  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(font_set, c, height);
    width += glyph->advance;
  }
  return width;
}

void ui_draw_text_centered(struct ui *ui, struct font_set *font_set, fvec2_t position, const char *str, unsigned height)
{
  position.x -= font_compute_text_width(font_set, str, height) * 0.5f;
  ui_draw_text(ui, font_set, position, str, height);
}
