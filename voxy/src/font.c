#include "font.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE int
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <stdio.h>

////////////////////////
/// Glyph Hash Table ///
////////////////////////
int glyph_key(struct glyph *glyph)
{
  return glyph->c;
}

size_t glyph_hash(int c)
{
  return c;
}

int glyph_compare(int c1, int c2)
{
  return c1 - c2;
}

void glyph_dispose(struct glyph *glyph)
{
  glDeleteTextures(1, &glyph->texture);
  free(glyph);
}

/////////////////
/// Free Type ///
/////////////////
static FT_Library ft_handle()
{
  static FT_Library ft;
  static int ft_initialized;
  if(!ft_initialized)
    FT_Init_FreeType(&ft);

  return ft;
}

#define FT_HANDLE ft_handle()

static const char *ft_strerror(FT_Error error)
{
  #undef FTERRORS_H_
  #define FT_ERROR_START_LIST     switch (error) {
  #define FT_ERRORDEF( e, v, s )    case v: return s;
  #define FT_ERROR_END_LIST       default: return "unknown"; }
  #include <freetype/fterrors.h>
}

////////////
/// Font ///
////////////
int font_load(struct font *font, const char *filepath)
{
  FT_Error error;

  if((error = FT_New_Face(FT_HANDLE, filepath, 0, &font->face)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to load font from %s: %s\n", filepath, ft_strerror(error));
    return -1;
  }

  if((error = FT_Set_Pixel_Sizes(font->face, 0, 48)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to load font from %s: %s\n", filepath, ft_strerror(error));
    return -1;
  }

  glyph_hash_table_init(&font->glyphs);
  return 0;
}

void font_unload(struct font *font)
{
  glyph_hash_table_dispose(&font->glyphs);
  FT_Done_Face(font->face);
}

struct glyph *font_get_glyph(struct font *font, int c)
{
  FT_Error error;

  struct glyph *glyph = glyph_hash_table_lookup(&font->glyphs, c);
  if(!glyph)
  {
    if((error = FT_Load_Char(font->face, c, FT_LOAD_RENDER)) != 0)
    {
      fprintf(stderr, "ERROR: Failed to load character %c: %s\n", c, ft_strerror(error));
      return NULL;
    }

    if((long)font->face->glyph->bitmap.width != (long)font->face->glyph->bitmap.pitch)
    {
      fprintf(stderr, "ERROR: Bitmap for glyph for character %c is not tightly packed\n", c);
      return NULL;
    }

    glyph = malloc(sizeof *glyph);
    glyph->c = c;

    glyph->dimension_x = font->face->glyph->metrics.width;
    glyph->dimension_y = font->face->glyph->metrics.height;

    glyph->bearing_x = font->face->glyph->metrics.horiBearingX;
    glyph->bearing_y = font->face->glyph->metrics.horiBearingY;

    glyph->advance = font->face->glyph->metrics.horiAdvance;

    glGenTextures(1, &glyph->texture);
    glBindTexture(GL_TEXTURE_2D, glyph->texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font->face->glyph->bitmap.width, font->face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, font->face->glyph->bitmap.buffer);
  }
  return glyph;
}

