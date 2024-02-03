#include "resource_pack.h"
#include "check.h"

#include <stdio.h>
#include <dlfcn.h>
#include <errno.h>
#include <string.h>


int resource_pack_load(struct resource_pack *resource_pack, const char *filepath)
{
  VOXY_CHECK_DECLARE(font_set);
  VOXY_CHECK_DECLARE(block_array_texture);

  size_t       filepath_count = 0;
  const char **filepaths      = NULL;

  if(!(resource_pack->handle = dlopen(filepath, RTLD_LAZY)))
  {
    fprintf(stderr, "ERROR: Failed to load resource pack from %s: %s\n", filepath, strerror(errno));
    goto error;
  }

#define LOAD_SYMBOL_FUNC(name)       do { if(!(resource_pack->name = dlsym(resource_pack->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } } while(0)
#define LOAD_SYMBOL_TYPE(name, type) do { type *value; if(!(value = dlsym(resource_pack->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } resource_pack->name = *value; } while(0)

  LOAD_SYMBOL_FUNC(block_infos);

  LOAD_SYMBOL_FUNC(block_texture_infos);
  LOAD_SYMBOL_TYPE(block_texture_info_count, size_t);

  LOAD_SYMBOL_FUNC(block_infos);
  LOAD_SYMBOL_TYPE(block_info_count, size_t);

  LOAD_SYMBOL_FUNC(generate_heights);
  LOAD_SYMBOL_FUNC(generate_tiles);

#undef LOAD_SYMBOL_FUNC
#undef LOAD_SYMBOL_TYPE

  filepath_count = resource_pack->block_texture_info_count;
  filepaths      = malloc(filepath_count * sizeof *filepaths);
  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;

  font_set_init(&resource_pack->font_set);
  font_set_load(&resource_pack->font_set, "assets/arial.ttf");
  font_set_load(&resource_pack->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&resource_pack->font_set);

  VOXY_CHECK_INIT(font_set,            0);
  VOXY_CHECK_INIT(block_array_texture, gl_array_texture_2d_load(&resource_pack->block_array_texture, filepath_count, filepaths));

  return 0;

error:
  VOXY_CHECK_FINI(font_set,            font_set_fini(&resource_pack->font_set));
  VOXY_CHECK_FINI(block_array_texture, gl_array_texture_2d_fini(&resource_pack->block_array_texture));

  if(resource_pack->handle)
    dlclose(resource_pack->handle);

  return -1;
}

void resource_pack_unload(struct resource_pack *resource_pack)
{
  dlclose(resource_pack->handle);
  font_set_fini(&resource_pack->font_set);
  gl_array_texture_2d_fini(&resource_pack->block_array_texture);
}

