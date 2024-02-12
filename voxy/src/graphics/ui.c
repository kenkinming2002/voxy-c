#include <voxy/graphics/ui.h>
#include <voxy/graphics/gl_programs.h>

#include <voxy/core/window.h>

static GLuint vao_instance;

static void vao_atexit(void)
{
  glDeleteVertexArrays(1, &vao_instance);
}

static GLuint vao_get(void)
{
  if(vao_instance == 0)
  {
    glCreateVertexArrays(1, &vao_instance);
    atexit(vao_atexit);
  }
  return vao_instance;
}

static void ui_setup(void)
{
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

static void ui_draw(void)
{
  GLuint vao = vao_get();
  glBindVertexArray(vao);
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

void ui_draw_quad(fvec2_t position, fvec2_t dimension, fvec4_t color)
{
  struct gl_program *program = gl_program_ui_quad_get();
  ui_setup();
  {
    glUseProgram(program->id);
    glUniform2f(glGetUniformLocation(program->id, "window_size"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(program->id, "position"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program->id, "dimension"), dimension.x, dimension.y);
    glUniform4f(glGetUniformLocation(program->id, "color"), color.r, color.g, color.b, color.a);
  }
  ui_draw();
}

void ui_draw_quad_rounded(fvec2_t position, fvec2_t dimension, float radius, fvec4_t color)
{
  struct gl_program *program = gl_program_ui_quad_rounded_get();
  ui_setup();
  {
    glUseProgram(program->id);
    glUniform2f(glGetUniformLocation(program->id, "window_size"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(program->id, "position"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program->id, "dimension"), dimension.x, dimension.y);
    glUniform1f(glGetUniformLocation(program->id, "radius"), radius);
    glUniform4f(glGetUniformLocation(program->id, "color"), color.r, color.g, color.b, color.a);
  }
  ui_draw();
}

void ui_draw_texture(fvec2_t position, fvec2_t dimension, GLuint texture)
{
  struct gl_program *program = gl_program_ui_texture_get();
  ui_setup();
  {
    glUseProgram(program->id);
    glUniform2f(glGetUniformLocation(program->id, "window_size"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(program->id, "position"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program->id, "dimension"), dimension.x, dimension.y);
    glBindTexture(GL_TEXTURE_2D, texture);
  }
  ui_draw();

}

void ui_draw_texture_mono(fvec2_t position, fvec2_t dimension, GLuint texture)
{
  struct gl_program *program = gl_program_ui_texture_mono_get();
  ui_setup();
  {
    glUseProgram(program->id);
    glUniform2f(glGetUniformLocation(program->id, "window_size"), window_size.x, window_size.y);
    glUniform2f(glGetUniformLocation(program->id, "position"), position.x, position.y);
    glUniform2f(glGetUniformLocation(program->id, "dimension"), dimension.x, dimension.y);
    glBindTexture(GL_TEXTURE_2D, texture);
  }
  ui_draw();
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

void ui_draw_text(struct font_set *font_set, fvec2_t position, const char *str, unsigned height)
{
  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(font_set, c, height);
    fvec2_t current_position  = fvec2_add(position, glyph->bearing);
    fvec2_t current_dimension = fvec2_mul(glyph->dimension, fvec2(1.0f, -1.0f));
    ui_draw_texture_mono(current_position, current_dimension, glyph->texture);
    position.x += glyph->advance;
  }
}

void ui_draw_text_centered(struct font_set *font_set, fvec2_t position, const char *str, unsigned height)
{
  position.x -= font_compute_text_width(font_set, str, height) * 0.5f;
  ui_draw_text(font_set, position, str, height);
}

