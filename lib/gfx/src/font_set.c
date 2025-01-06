#include <libgfx/font_set.h>

#include <libcore/log.h>

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX glyph
#define SC_HASH_TABLE_NODE_TYPE struct glyph
#define SC_HASH_TABLE_KEY_TYPE struct glyph_key
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <fontconfig/fontconfig.h>

////////////////////////
/// Glyph Hash Table ///
////////////////////////
struct glyph_key glyph_key(struct glyph *glyph)
{
  return glyph->key;
}

size_t glyph_hash(struct glyph_key key)
{
  return key.c * 13 + key.height * 19 + key.outline * 31;
}

int glyph_compare(struct glyph_key key1, struct glyph_key key2)
{
  if(key1.c < key2.c) return -1;
  if(key1.c > key2.c) return +1;

  if(key1.height < key2.height) return -1;
  if(key1.height > key2.height) return +1;

  if(key1.outline < key2.outline) return -1;
  if(key1.outline > key2.outline) return +1;

  return 0;
}

void glyph_dispose(struct glyph *glyph)
{
  glDeleteTextures(1, &glyph->interior_texture);
  glDeleteTextures(1, &glyph->outline_texture);
  free(glyph);
}

/////////////////
/// Free Type ///
/////////////////
static FT_Library _ft_library;

static void ft_library_atexit()
{
  FT_Done_FreeType(_ft_library);
}

static FT_Library ft_library(void)
{
  if(!_ft_library)
  {
    FT_Init_FreeType(&_ft_library);
    atexit(ft_library_atexit);
  }
  return _ft_library;
}

static FT_Stroker _ft_stroker;

static void ft_stroker_atexit()
{
  FT_Stroker_Done(_ft_stroker);
}

static FT_Stroker ft_stroker(void)
{
  if(!_ft_stroker)
  {
    FT_Stroker_New(ft_library(), &_ft_stroker);
    atexit(ft_stroker_atexit);
  }
  return _ft_stroker;
}

#define FT_LIBRARY ft_library()
#define FT_STROKER ft_stroker()

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
  if((error = FT_New_Face(FT_LIBRARY, filepath, 0, &font.face)) != 0)
  {
    LOG_WARN("Failed to load font from %s: %s", filepath, ft_strerror(error));
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
struct glyph *font_set_get_glyph(struct font_set *font_set, unsigned c, unsigned height, unsigned outline)
{
  struct glyph_key glyph_key = { .c = c, .height = height, .outline = outline, };
  struct glyph *glyph = glyph_hash_table_lookup(&font_set->glyphs, glyph_key);
  if(glyph)
    return glyph;

  glyph = malloc(sizeof *glyph);
  glyph->key = glyph_key;

  for(size_t i=0; i<font_set->font_count; ++i)
  {
    FT_Error error;

    // TODO: Use FT_Set_Char_Size and handle DPI scaling
    if((error = FT_Set_Pixel_Sizes(font_set->fonts[i].face, 0, height)) != 0)
    {
      LOG_WARN("Failed to set pixel size: Font %s: %s", font_set->fonts[i].filepath, ft_strerror(error));
      continue;
    }

    if((error = FT_Load_Char(font_set->fonts[i].face, c, FT_LOAD_TARGET_NORMAL | FT_LOAD_NO_BITMAP)) != 0)
    {
      LOG_WARN("Failed to load character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
      continue;
    }

    // Render the outline
    {
      FT_Glyph ft_glyph;
      if((error = FT_Get_Glyph(font_set->fonts[i].face->glyph, &ft_glyph)) != 0)
      {
        LOG_WARN("Failed to get glyph for character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
        continue;
      }

      FT_Stroker_Set(FT_STROKER, (FT_Fixed)outline << 6, FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
      if((error = FT_Glyph_Stroke(&ft_glyph, FT_STROKER, true)) != 0)
      {
        LOG_WARN("Failed to get glyph for character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
        FT_Done_Glyph(ft_glyph);
        continue;
      }

      if((error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, NULL, true)) != 0)
      {
        LOG_WARN("Failed to get glyph for character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
        FT_Done_Glyph(ft_glyph);
        continue;
      }

      FT_BitmapGlyph ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
      FT_Bitmap *ft_bitmap = &ft_bitmap_glyph->bitmap;

      glGenTextures(1, &glyph->outline_texture);
      glBindTexture(GL_TEXTURE_2D, glyph->outline_texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ZERO);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ZERO);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ZERO);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ft_bitmap->width, ft_bitmap->rows, 0, GL_RED, GL_UNSIGNED_BYTE, ft_bitmap->buffer);

      glyph->outline_dimension = fvec2(ft_bitmap->width, ft_bitmap->rows);
      glyph->outline_bearing = fvec2(ft_bitmap_glyph->left, ft_bitmap_glyph->top);

      FT_Done_Glyph(ft_glyph);
    }

    // Render the interior
    {
      FT_Glyph ft_glyph;
      if((error = FT_Get_Glyph(font_set->fonts[i].face->glyph, &ft_glyph)) != 0)
      {
        LOG_WARN("Failed to get glyph for character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
        continue;
      }

      if((error = FT_Glyph_To_Bitmap(&ft_glyph, FT_RENDER_MODE_NORMAL, NULL, true)) != 0)
      {
        LOG_WARN("Failed to get glyph for character %c: Font %s: %s", c, font_set->fonts[i].filepath, ft_strerror(error));
        FT_Done_Glyph(ft_glyph);
        continue;
      }

      FT_BitmapGlyph ft_bitmap_glyph = (FT_BitmapGlyph)ft_glyph;
      FT_Bitmap *ft_bitmap = &ft_bitmap_glyph->bitmap;

      glGenTextures(1, &glyph->interior_texture);
      glBindTexture(GL_TEXTURE_2D, glyph->interior_texture);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R, GL_ONE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G, GL_ONE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B, GL_ONE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A, GL_RED);

      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, ft_bitmap->width, ft_bitmap->rows, 0, GL_RED, GL_UNSIGNED_BYTE, ft_bitmap->buffer);

      glyph->interior_dimension = fvec2(ft_bitmap->width, ft_bitmap->rows);
      glyph->interior_bearing = fvec2(ft_bitmap_glyph->left, ft_bitmap_glyph->top);

      // It would be nice if somebody to point to the documentation in freetype that say that it is in 16.16 format.
      glyph->advance = ft_glyph->advance.x / (float)(1 << 16);

      FT_Done_Glyph(ft_glyph);
    }

    /// Cache the rendered glpyh and return
    glyph_hash_table_insert_unchecked(&font_set->glyphs, glyph);
    return glyph;
  }

  LOG_WARN("No font found for character %c", c);
  free(glyph);
  return NULL;
}

