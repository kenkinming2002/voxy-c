#ifndef LIBGFX_FONT_SET_H
#define LIBGFX_FONT_SET_H

#include <glad/glad.h>
#include <libmath/vector.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>

struct glyph_key
{
  unsigned c;
  unsigned height;
  unsigned outline;
};

struct glyph_value
{
  GLuint outline_texture;
  fvec2_t outline_dimension;
  fvec2_t outline_bearing;

  GLuint interior_texture;
  fvec2_t interior_dimension;
  fvec2_t interior_bearing;

  float advance;
};

struct glyph
{
  struct glyph_key key;
  struct glyph_value value;
};

struct font
{
  char    *filepath;
  FT_Face  face;
};

struct font_set
{
  struct font *fonts;
  size_t       font_count;
  size_t       font_capacity;

  struct glyph *glyphs;
};

int font_load(struct font *font, const char *filepath);
void font_unload(struct font *font);

void font_set_init(struct font_set *font_set);
void font_set_fini(struct font_set *font_set);
int font_set_load(struct font_set *font_set, const char *filepath);
int font_set_load_system(struct font_set *font_set);

struct glyph *font_set_get_glyph(struct font_set *font_set, unsigned c, unsigned height, unsigned outline);

#endif // LIBGFX_FONT_SET_H
