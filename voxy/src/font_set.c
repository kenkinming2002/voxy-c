#include "font_set.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE struct glyph_key
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
struct glyph_key glyph_key(struct glyph *glyph)
{
  return glyph->key;
}

size_t glyph_hash(struct glyph_key key)
{
  return key.c * 13 + key.height * 19;
}

int glyph_compare(struct glyph_key key1, struct glyph_key key2)
{
  if(key1.c < key2.c) return -1;
  if(key1.c > key2.c) return +1;

  if(key1.height < key2.height) return -1;
  if(key1.height > key2.height) return +1;

  return 0;
}

void glyph_dispose(struct glyph *glyph)
{
  glDeleteTextures(1, &glyph->texture);
  free(glyph);
}

/////////////////
/// Free Type ///
/////////////////
static FT_Library ft;

static void ft_atexit()
{
  FT_Done_FreeType(ft);
}

static FT_Library ft_handle()
{
  if(!ft)
  {
    FT_Init_FreeType(&ft);
    atexit(ft_atexit);
  }
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
  {
    fc_initialized = FcInit();
    atexit(FcFini);
  }

  return fc_initialized ? 0 : -1;
}

////////////////
/// Font Set ///
////////////////
void font_set_init(struct font_set *font_set)
{
  font_set->fonts         = NULL;
  font_set->font_count    = 0;
  font_set->font_capacity = 0;
  glyph_hash_table_init(&font_set->glyphs);
}

void font_set_fini(struct font_set *font_set)
{
  glyph_hash_table_dispose(&font_set->glyphs);
  for(size_t i=0; i<font_set->font_count; ++i)
  {
    free(font_set->fonts[i].filepath);
    FT_Done_Face(font_set->fonts[i].face);
  }
  free(font_set->fonts);
}

int font_set_load(struct font_set *font_set, const char *filepath)
{
  FT_Error error;

  struct font font;
  font.filepath = strdup(filepath); // FIXME: Non-POSIX platforms
  if((error = FT_New_Face(FT_HANDLE, filepath, 0, &font.face)) != 0)
  {
    fprintf(stderr, "ERROR: Failed to load font from %s: %s\n", filepath, ft_strerror(error));
    return -1;
  }

  if(font_set->font_capacity == font_set->font_count)
  {
    font_set->font_capacity = font_set->font_capacity != 0 ? font_set->font_capacity * 2 : 1;
    font_set->fonts         = realloc(font_set->fonts, font_set->font_capacity * sizeof *font_set->fonts);
  }
  font_set->fonts[font_set->font_count++] = font;
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
  FcFontSetDestroy(fc_font_set);
  FcObjectSetDestroy(fc_object_set);
  FcPatternDestroy(fc_pattern);
  return 0;
}

/////////////
/// Glyph ///
/////////////
struct glyph *font_set_get_glyph(struct font_set *font_set, unsigned c, unsigned height)
{
  struct glyph_key glyph_key = { .c = c, .height = height};
  struct glyph *glyph = glyph_hash_table_lookup(&font_set->glyphs, glyph_key);
  if(!glyph)
    for(size_t i=0; i<font_set->font_count; ++i)
    {
      FT_UInt char_index;
      if((char_index = FT_Get_Char_Index(font_set->fonts[i].face, c)) == 0)
        continue;

      FT_Error error;

      // TODO: Use FT_Set_Char_Size and handle DPI scaling
      if((error = FT_Set_Pixel_Sizes(font_set->fonts[i].face, 0, height)) != 0)
      {
        fprintf(stderr, "WARN: Failed to set pixel size: Font %s: %s\n", font_set->fonts[i].filepath, ft_strerror(error));
        continue;
      }

      if((error = FT_Load_Glyph(font_set->fonts[i].face, char_index, FT_LOAD_RENDER)) != 0)
      {
        fprintf(stderr, "WARN: Failed to load character %c: Font %s: %s\n", c, font_set->fonts[i].filepath, ft_strerror(error));
        continue;
      }

      if((long)font_set->fonts[i].face->glyph->bitmap.width != (long)font_set->fonts[i].face->glyph->bitmap.pitch)
      {
        fprintf(stderr, "WARN: Bitmap is not tightly packed for character %c: Font %s\n", c, font_set->fonts[i].filepath);
        continue;
      }

      glyph = malloc(sizeof *glyph);
      glyph->key = glyph_key;

      glyph->dimension = fvec2(font_set->fonts[i].face->glyph->bitmap.width, font_set->fonts[i].face->glyph->bitmap.rows);
      glyph->bearing   = fvec2(font_set->fonts[i].face->glyph->bitmap_left, font_set->fonts[i].face->glyph->bitmap_top);
      glyph->advance   = font_set->fonts[i].face->glyph->advance.x / 64.0f;

      glGenTextures(1, &glyph->texture);
      glBindTexture(GL_TEXTURE_2D, glyph->texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, font_set->fonts[i].face->glyph->bitmap.width, font_set->fonts[i].face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, font_set->fonts[i].face->glyph->bitmap.buffer);

      glyph_hash_table_insert_unchecked(&font_set->glyphs, glyph);
    }

  if(!glyph)
  {
    fprintf(stderr, "ERROR: No font found for character %c\n", c);
    return NULL;
  }

  return glyph;
}

