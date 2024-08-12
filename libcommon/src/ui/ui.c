#include <libcommon/ui/ui.h>

#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/font_set.h>
#include <libcommon/core/window.h>
#include <libcommon/utils/dynamic_array.h>

#include <assert.h>

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

static DYNAMIC_ARRAY_DECLARE(colored_quads, struct colored_quad);
static DYNAMIC_ARRAY_DECLARE(textured_quads, struct textured_quad);

void ui_reset(void)
{
  colored_quads.item_count = 0;
  textured_quads.item_count = 0;
}

void ui_quad_colored(fvec2_t position, fvec2_t dimension, float rounding, fvec4_t color)
{
  struct colored_quad colored_quad;
  colored_quad.position = position;
  colored_quad.dimension = dimension;
  colored_quad.rounding = rounding;
  colored_quad.color = color;
  DYNAMIC_ARRAY_APPEND(colored_quads, colored_quad);
}

void ui_rect_textured(fvec2_t position, fvec2_t dimension, float rounding, GLuint texture)
{
  struct textured_quad textured_quad;
  textured_quad.position = position;
  textured_quad.dimension = dimension;
  textured_quad.rounding = rounding;
  textured_quad.texture = texture;
  DYNAMIC_ARRAY_APPEND(textured_quads, textured_quad);
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
    font_set_load(&font_set, "libcommon/assets/fonts/arial.ttf");
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
    struct glyph *glyph = font_set_get_glyph(&font_set, c, height, 1);
    width += glyph->advance;
  }
  return width;
}

void ui_text(fvec2_t position, unsigned height, unsigned outline, const char *str)
{
  font_set_ensure();

  int c;
  while((c = utf8_next((const unsigned char **)&str)))
  {
    struct glyph *glyph = font_set_get_glyph(&font_set, c, height, outline);

    {
      fvec2_t current_position  = fvec2_add(position, glyph->outline_bearing);
      fvec2_t current_dimension = fvec2_mul(glyph->outline_dimension, fvec2(1.0f, -1.0f));
      ui_rect_textured(current_position, current_dimension, 0.0f, glyph->outline_texture);
    }

    {
      fvec2_t current_position  = fvec2_add(position, glyph->interior_bearing);
      fvec2_t current_dimension = fvec2_mul(glyph->interior_dimension, fvec2(1.0f, -1.0f));
      ui_rect_textured(current_position, current_dimension, 0.0f, glyph->interior_texture);
    }

    position.x += glyph->advance;
  }
}

void ui_text_centered(float y, unsigned height, unsigned outline, const char *str)
{
  fvec2_t position;
  position.x = (window_size.x - ui_text_width(height, str)) * 0.5f;
  position.y = y;
  ui_text(position, height, outline, str);
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
    struct gl_program program = GL_PROGRAM_LOAD(libcommon/assets/shaders/ui/quad_rounded);
    glUseProgram(program.id);
    glUniform2f(glGetUniformLocation(program.id, "window_size"), window_size.x, window_size.y);
    for(size_t i=0; i<colored_quads.item_count; ++i)
    {
      struct colored_quad quad = colored_quads.items[i];

      glUniform2f(glGetUniformLocation(program.id, "position"), quad.position.x, quad.position.y);
      glUniform2f(glGetUniformLocation(program.id, "dimension"), quad.dimension.x, quad.dimension.y);
      glUniform1f(glGetUniformLocation(program.id, "radius"), quad.rounding);
      glUniform4f(glGetUniformLocation(program.id, "color"), quad.color.r, quad.color.g, quad.color.b, quad.color.a);
      glDrawArrays(GL_TRIANGLES, 0, 6);
    }
  }

  // Textured
  {
    struct gl_program program = GL_PROGRAM_LOAD(libcommon/assets/shaders/ui/quad_textured);
    glUseProgram(program.id);
    glUniform2f(glGetUniformLocation(program.id, "window_size"), window_size.x, window_size.y);
    for(size_t i=0; i<textured_quads.item_count; ++i)
    {
      struct textured_quad quad = textured_quads.items[i];

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

