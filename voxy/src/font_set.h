#ifndef VOXY_FONT_SET_H
#define VOXY_FONT_SET_H

#include "glad/glad.h"
#include "lin.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE int
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#include <ft2build.h>
#include FT_FREETYPE_H

struct glyph
{
  struct glyph *next;
  size_t        hash;

  int c;

  struct vec2 dimension;
  struct vec2 bearing;
  float advance;

  GLuint texture;
};

struct font
{
  FT_Face                 face;
  struct glyph_hash_table glyphs;
};

struct font_set
{
  FT_Face *faces;
  size_t   face_count;
  size_t   face_capacity;

  struct glyph_hash_table glyphs;
};

int font_load(struct font *font, const char *filepath);
void font_unload(struct font *font);

void font_set_init(struct font_set *font_set);
void font_set_deinit(struct font_set *font_set);
int font_set_load(struct font_set *font_set, const char *filepath);
int font_set_load_system(struct font_set *font_set);

struct glyph *font_get_glyph(struct font *font, int c);
struct glyph *font_set_get_glyph(struct font_set *font_set, int c);

#endif // VOXY_FONT_SET_H
