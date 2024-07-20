#include <voxy/graphics/ui.h>

#include <voxy/graphics/gl.h>
#include <voxy/graphics/font_set.h>

#include <voxy/core/window.h>

#include <assert.h>

#define MAX_QUADS 256

struct colored_quad
{
  fvec2_t position;
  fvec2_t dimension;
  float   rounding;
  fvec4_t color;
};

struct textured_quad
{
  fvec2_t position;
  fvec2_t dimension;
  float   rounding;
  GLuint  texture;
};

static struct colored_quad colored_quads[MAX_QUADS];
static size_t              colored_quad_count;

static struct textured_quad textured_quads[MAX_QUADS];
static size_t               textured_quad_count;

void ui_reset(void)
{
  colored_quad_count  = 0;
  textured_quad_count = 0;
}

void ui_quad_colored(fvec2_t position, fvec2_t dimension, float rounding, fvec4_t color)
{
  assert(colored_quad_count < MAX_QUADS);

  colored_quads[colored_quad_count].position  = position;
  colored_quads[colored_quad_count].dimension = dimension;
  colored_quads[colored_quad_count].rounding  = rounding;
  colored_quads[colored_quad_count].color     = color;

  colored_quad_count += 1;
}

void ui_rect_textured(fvec2_t position, fvec2_t dimension, float rounding, GLuint texture)
{
  assert(textured_quad_count < MAX_QUADS);

  textured_quads[textured_quad_count].position  = position;
  textured_quads[textured_quad_count].dimension = dimension;
  textured_quads[textured_quad_count].rounding  = rounding;
  textured_quads[textured_quad_count].texture   = texture;

  textured_quad_count += 1;
}

static struct font_set font_set;

static void font_set_atexit(void)
{
  font_set_fini(&font_set);
}

static void font_set_ensure(void)
{
  if(!font_set.fonts)
  {
    font_set_load(&font_set, "assets/fonts/arial.ttf");
    font_set_load(&font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
    font_set_load_system(&font_set);
    atexit(font_set_atexit);
  }
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

float ui_text_width(unsigned height, const char *str)
{
  font_set_ensure();

  float width = 0.0f;
  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(&font_set, c, height);
    width += glyph->advance;
  }
  return width;
}

void ui_text(fvec2_t position, unsigned height, const char *str)
{
  font_set_ensure();

  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(&font_set, c, height);
    fvec2_t current_position  = fvec2_add(position, glyph->bearing);
    fvec2_t current_dimension = fvec2_mul(glyph->dimension, fvec2(1.0f, -1.0f));
    ui_rect_textured(current_position, current_dimension, 0.0f, glyph->texture);
    position.x += glyph->advance;
  }
}

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

void ui_render(void)
{
  glEnable(GL_BLEND);
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindVertexArray(vao_get());

  // Colored
  {
    struct gl_program program = GL_PROGRAM_LOAD(ui_quad_rounded);
    glUseProgram(program.id);
    glUniform2f(glGetUniformLocation(program.id, "window_size"), window_size.x, window_size.y);
    for(size_t i=0; i<colored_quad_count; ++i)
    {
      struct colored_quad quad = colored_quads[i];

      glUniform2f(glGetUniformLocation(program.id, "position"), quad.position.x, quad.position.y);
      glUniform2f(glGetUniformLocation(program.id, "dimension"), quad.dimension.x, quad.dimension.y);
      glUniform1f(glGetUniformLocation(program.id, "radius"), quad.rounding);
      glUniform4f(glGetUniformLocation(program.id, "color"), quad.color.r, quad.color.g, quad.color.b, quad.color.a);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
  }

  // Textured
  {
    struct gl_program program = GL_PROGRAM_LOAD(ui_texture);
    glUseProgram(program.id);
    glUniform2f(glGetUniformLocation(program.id, "window_size"), window_size.x, window_size.y);
    for(size_t i=0; i<textured_quad_count; ++i)
    {
      struct textured_quad quad = textured_quads[i];

      glUniform2f(glGetUniformLocation(program.id, "position"), quad.position.x, quad.position.y);
      glUniform2f(glGetUniformLocation(program.id, "dimension"), quad.dimension.x, quad.dimension.y);
      glUniform1f(glGetUniformLocation(program.id, "radius"), quad.rounding);
      glBindTexture(GL_TEXTURE_2D, quad.texture);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
  }
}

int ui_button(fvec2_t position, fvec2_t dimension)
{
  int result = 0;
  if(position.x <= mouse_position.x && mouse_position.x <= position.x + dimension.x)
    if(position.y <= mouse_position.y && mouse_position.y <= position.y + dimension.y)
    {
      result |= UI_BUTTON_RESULT_HOVERED;
      if(input_press(BUTTON_LEFT))   result |= UI_BUTTON_RESULT_CLICK_LEFT;
      if(input_press(BUTTON_RIGHT))  result |= UI_BUTTON_RESULT_CLICK_RIGHT;
      if(input_press(BUTTON_MIDDLE)) result |= UI_BUTTON_RESULT_CLICK_MIDDLE;
    }

  return result;
}

