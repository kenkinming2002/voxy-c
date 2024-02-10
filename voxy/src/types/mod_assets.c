#include <types/mod_assets.h>

#include <voxy/mod_interface.h>

int mod_assets_load(struct mod_assets *mod_assets, struct mod *mod)
{
  ////////////////
  /// Font Set ///
  ////////////////
  font_set_init(&mod_assets->font_set);
  font_set_load(&mod_assets->font_set, "assets/arial.ttf");
  font_set_load(&mod_assets->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&mod_assets->font_set);

  ///////////////////////////
  /// Block Array Texture ///
  ///////////////////////////
  size_t       filepath_count = mod->block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);
  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = mod->block_texture_infos[i].filepath;

  if(gl_array_texture_2d_load(&mod_assets->block_array_texture, filepath_count, filepaths) != 0)
    goto error_block_array_texture;

  /////////////////////
  /// Item Textures ///
  /////////////////////
  mod_assets->item_texture_count = mod->item_info_count;
  mod_assets->item_textures      = malloc(mod->item_info_count * sizeof *mod_assets->item_textures);
  for(size_t i=0; i<mod_assets->item_texture_count; ++i)
    if(gl_texture_2d_load(&mod_assets->item_textures[i], mod->item_infos[i].texture_filepath) != 0)
    {
      mod_assets->item_texture_count = i;
      goto error_item_textures;
    }

  return 0;

  //////////////////////
  /// Error Handling ///
  //////////////////////
error_item_textures:
  for(size_t i=0; i<mod_assets->item_texture_count; ++i)
    gl_texture_2d_fini(&mod_assets->item_textures[i]);
  free(mod_assets->item_textures);

error_block_array_texture:
  font_set_fini(&mod_assets->font_set);

  return -1;
}

void mod_assets_unload(struct mod_assets *mod_assets)
{
  font_set_fini(&mod_assets->font_set);
  gl_array_texture_2d_fini(&mod_assets->block_array_texture);
  for(size_t i=0; i<mod_assets->item_texture_count; ++i)
    gl_texture_2d_fini(&mod_assets->item_textures[i]);
  free(mod_assets->item_textures);
}
