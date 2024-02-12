#include <main_game/mod_assets.h>
#include <main_game/mod.h>

#include <graphics/font_set.h>
#include <graphics/gl.h>

#include <stdlib.h>

static struct font_set             mod_assets_font_set;
static struct gl_array_texture_2d  mod_assets_block_array_texture;
static struct gl_texture_2d       *mod_assets_item_textures;
static size_t                      mod_assets_item_texture_count;

static void mod_assets_font_set_atexit(void)
{
  font_set_fini(&mod_assets_font_set);
}

static void mod_assets_block_array_texture_atexit(void)
{
  gl_array_texture_2d_fini(&mod_assets_block_array_texture);
}

static void mod_assets_item_textures_atexit(void)
{
  for(size_t i=0; i<mod_assets_item_texture_count; ++i)
    gl_texture_2d_fini(&mod_assets_item_textures[i]);
}

struct font_set *mod_assets_font_set_get(void)
{
  if(!mod_assets_font_set.fonts)
  {
    font_set_load(&mod_assets_font_set, "assets/arial.ttf");
    font_set_load(&mod_assets_font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
    font_set_load_system(&mod_assets_font_set);
    atexit(mod_assets_font_set_atexit);
  }
  return &mod_assets_font_set;
}

struct gl_array_texture_2d *mod_assets_block_array_texture_get(void)
{
  if(mod_assets_block_array_texture.id == 0)
  {
    size_t       filepath_count = mod_block_texture_info_count_get();
    const char **filepaths      = malloc(filepath_count * sizeof *filepaths);
    for(size_t i=0; i<filepath_count; ++i)
      filepaths[i] = mod_block_texture_info_get(i)->filepath;

    if(gl_array_texture_2d_load(&mod_assets_block_array_texture, filepath_count, filepaths) != 0)
    {
      fprintf(stderr, "Failed to load block array texture");
      exit(EXIT_FAILURE);
    }

    free(filepaths);
    atexit(mod_assets_block_array_texture_atexit);
  }
  return &mod_assets_block_array_texture;
}

struct gl_texture_2d *mod_assets_item_texture_get(uint8_t item_id)
{
  if(!mod_assets_item_textures)
  {
    mod_assets_item_texture_count = mod_item_info_count_get();
    mod_assets_item_textures      = malloc(mod_assets_item_texture_count * sizeof *mod_assets_item_textures);
    for(size_t i=0; i<mod_assets_item_texture_count; ++i)
    {
      const struct item_info *item_info = mod_item_info_get(i);
      if(gl_texture_2d_load(&mod_assets_item_textures[i], item_info->texture_filepath) != 0)
      {
        fprintf(stderr, "Failed to load item texture %zu\n", i);
        exit(EXIT_FAILURE);
      }
    }
    atexit(mod_assets_item_textures_atexit);
  }
  return &mod_assets_item_textures[item_id];
}
