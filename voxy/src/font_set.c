#include "font_set.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE int
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <fontconfig/fontconfig.h>

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

///////////////////
/// Font Config ///
///////////////////
static int fc_ensure_init()
{
  static int fc_initialized;
  if(!fc_initialized)
    fc_initialized = FcInit();

  return fc_initialized ? 0 : -1;
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

  // TODO: Use FT_Set_Char_Size and handle DPI scaling
  if((error = FT_Set_Pixel_Sizes(font->face, 0, 24)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to set pixel size for font %s: %s\n", filepath, ft_strerror(error));
    FT_Done_Face(font->face);
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

////////////////
/// Font Set ///
////////////////
void font_set_init(struct font_set *font_set)
{
  font_set->faces         = NULL;
  font_set->face_count    = 0;
  font_set->face_capacity = 0;
  glyph_hash_table_init(&font_set->glyphs);
}

void font_set_deinit(struct font_set *font_set)
{
  glyph_hash_table_dispose(&font_set->glyphs);
  for(size_t i=0; i<font_set->face_count; ++i)
    FT_Done_Face(font_set->faces[i]);
  free(font_set->faces);
}

int font_set_load(struct font_set *font_set, const char *filepath)
{
  FT_Error error;
  FT_Face  face;

  if((error = FT_New_Face(FT_HANDLE, filepath, 0, &face)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to load font from %s: %s\n", filepath, ft_strerror(error));
    return -1;
  }

  // TODO: Use FT_Set_Char_Size and handle DPI scaling
  if((error = FT_Set_Pixel_Sizes(face, 0, 24)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to set pixel size for font %s: %s\n", filepath, ft_strerror(error));
    FT_Done_Face(face);
    return -1;
  }

  if(font_set->face_capacity == font_set->face_count)
  {
    font_set->face_capacity = font_set->face_capacity != 0 ? font_set->face_capacity * 2 : 1;
    font_set->faces         = realloc(font_set->faces, font_set->face_capacity * sizeof *font_set->faces);
  }
  font_set->faces[font_set->face_count++] = face;
  return 0;
}

int font_set_load_system(struct font_set *font_set)
{
  if(fc_ensure_init() != 0)
    return -1;

  FcConfig *fc_config = FcConfigGetCurrent();
  FcConfigSetRescanInterval(fc_config, 0);

  FcPattern   *fc_pattern    = FcPatternCreate();
  FcObjectSet *fc_object_set = FcObjectSetBuild(FC_FILE, NULL);
  FcFontSet   *fc_font_set   = FcFontList(fc_config, fc_pattern, fc_object_set);
  for(int i=0; i<fc_font_set->nfont; ++i)
  {
    FcChar8 *file;
    if(FcPatternGetString(fc_font_set->fonts[i], FC_FILE, 0, &file) == FcResultMatch)
      font_set_load(font_set, (const char *)file);
  }
  return 0;
}

/////////////
/// Glyph ///
/////////////
struct glyph *font_set_get_glyph(struct font_set *font_set, int c)
{

  struct glyph *glyph = glyph_hash_table_lookup(&font_set->glyphs, c);
  if(!glyph)
    for(size_t i=0; i<font_set->face_count; ++i)
    {
      FT_UInt char_index;
      if((char_index = FT_Get_Char_Index(font_set->faces[i], c)) == 0)
        continue;

      FT_Error error;
      if((error = FT_Load_Glyph(font_set->faces[i], char_index, FT_LOAD_RENDER)) != 0)
      {
        fprintf(stderr, "WARN: Failed to load character %c from font: %s\n", c, ft_strerror(error));
        continue;
      }

      if((long)font_set->faces[i]->glyph->bitmap.width != (long)font_set->faces[i]->glyph->bitmap.pitch)
      {
        fprintf(stderr, "WARN: Bitmap is not tightly packed for character %c\n", c);
        continue;
      }

      glyph = malloc(sizeof *glyph);
      glyph->c = c;

      glyph->dimension = vec2(font_set->faces[i]->glyph->bitmap.width, font_set->faces[i]->glyph->bitmap.rows);
      glyph->bearing   = vec2(font_set->faces[i]->glyph->bitmap_left, font_set->faces[i]->glyph->bitmap_top);
      glyph->advance   = font_set->faces[i]->glyph->advance.x / 64.0f;

      glGenTextures(1, &glyph->texture);
      glBindTexture(GL_TEXTURE_2D, glyph->texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_set->faces[i]->glyph->bitmap.width, font_set->faces[i]->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, font_set->faces[i]->glyph->bitmap.buffer);

      return glyph;
    }

  return NULL;
}

