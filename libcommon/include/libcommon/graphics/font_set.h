#ifndef LIBCOMMON_GRAPHICS_FONT_SET_H
#define LIBCOMMON_GRAPHICS_FONT_SET_H

#include "glad/glad.h"
#include <libcommon/math/vector.h>

#include <ft2build.h>
#include FT_FREETYPE_H

struct glyph_key
{
  unsigned c;
  unsigned height;
};

struct glyph
{
  struct glyph *next;
  size_t        hash;

  struct glyph_key key;
  fvec2_t      dimension;
  fvec2_t      bearing;
  float            advance;
  GLuint           texture;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE struct glyph_key
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

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

  struct glyph_hash_table glyphs;
};

int font_load(struct font *font, const char *filepath);
void font_unload(struct font *font);

void font_set_init(struct font_set *font_set);
void font_set_fini(struct font_set *font_set);
int font_set_load(struct font_set *font_set, const char *filepath);
int font_set_load_system(struct font_set *font_set);

struct glyph *font_set_get_glyph(struct font_set *font_set, unsigned c, unsigned height);

#endif // LIBCOMMON_GRAPHICS_FONT_SET_H
