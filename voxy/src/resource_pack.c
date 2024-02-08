#include "resource_pack.h"
#include "check.h"

#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>

#define LOAD_SYMBOL_ADDR(name)       do { if(!(resource_pack->name = dlsym(resource_pack->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } } while(0)
#define LOAD_SYMBOL_TYPE(name, type) do { type *value; if(!(value = dlsym(resource_pack->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } resource_pack->name = *value; } while(0)

int resource_pack_load(struct resource_pack *resource_pack, const char *filepath)
{
  VOXY_CHECK_DECLARE(font_set);
  VOXY_CHECK_DECLARE(block_array_texture);
  VOXY_CHECK_ARRAY_DECLARE(item_textures);

  size_t       filepath_count = 0;
  const char **filepaths      = NULL;

  resource_pack->handle        = NULL;
  resource_pack->item_textures = NULL;

  if(!(resource_pack->handle = dlopen(filepath, RTLD_LAZY)))
  {
    fprintf(stderr, "ERROR: Failed to load resource pack from %s: %s\n", filepath, strerror(errno));
    goto error;
  }

  LOAD_SYMBOL_ADDR(block_texture_infos);
  LOAD_SYMBOL_TYPE(block_texture_info_count, size_t);

  LOAD_SYMBOL_ADDR(block_infos);
  LOAD_SYMBOL_TYPE(block_info_count, size_t);

  LOAD_SYMBOL_ADDR(item_infos);
  LOAD_SYMBOL_TYPE(item_info_count, size_t);

  LOAD_SYMBOL_ADDR(generate_blocks);
  LOAD_SYMBOL_ADDR(generate_spawn);

  filepath_count = resource_pack->block_texture_info_count;
  filepaths      = malloc(filepath_count * sizeof *filepaths);
  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;

  resource_pack->item_textures = malloc(resource_pack->item_info_count * sizeof *resource_pack->item_textures);

  font_set_init(&resource_pack->font_set);
  font_set_load(&resource_pack->font_set, "assets/arial.ttf");
  font_set_load(&resource_pack->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&resource_pack->font_set);

  VOXY_CHECK_INIT(font_set,            0);
  VOXY_CHECK_INIT(block_array_texture, gl_array_texture_2d_load(&resource_pack->block_array_texture, filepath_count, filepaths));
  VOXY_CHECK_ARRAY_INIT(item_textures, gl_texture_2d_load(&resource_pack->item_textures[i], resource_pack->item_infos[i].texture_filepath), resource_pack->item_info_count);

  return 0;

error:
  VOXY_CHECK_FINI(font_set,            font_set_fini(&resource_pack->font_set));
  VOXY_CHECK_FINI(block_array_texture, gl_array_texture_2d_fini(&resource_pack->block_array_texture));
  VOXY_CHECK_ARRAY_FINI(item_textures, gl_texture_2d_fini(&resource_pack->item_textures[i]));

  if(resource_pack->item_textures) free(resource_pack->item_textures);
  if(resource_pack->handle)        dlclose(resource_pack->handle);

  return -1;
}

void resource_pack_unload(struct resource_pack *resource_pack)
{
  VOXY_FINI(font_set_fini(&resource_pack->font_set));
  VOXY_FINI(gl_array_texture_2d_fini(&resource_pack->block_array_texture));
  VOXY_ARRAY_FINI(gl_texture_2d_fini(&resource_pack->item_textures[i]), resource_pack->item_info_count);

  free(resource_pack->item_textures);
  dlclose(resource_pack->handle);
}

