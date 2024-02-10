#include <types/mod.h>

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#define LOAD_SYMBOL_ADDR(name)       do { if(!(mod->name = dlsym(mod->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } } while(0)
#define LOAD_SYMBOL_TYPE(name, type) do { type *value; if(!(value = dlsym(mod->handle, #name))) { fprintf(stderr, "ERROR: Missing symbol %s from resource pack %s\n", #name, filepath); goto error; } mod->name = *value; } while(0)

int mod_load(struct mod *mod, const char *filepath)
{
  if(!(mod->handle = dlopen(filepath, RTLD_LAZY)))
  {
    fprintf(stderr, "ERROR: Failed to open mod from %s: %s\n", filepath, strerror(errno));
    return -1;
  }

  LOAD_SYMBOL_ADDR(block_texture_infos);
  LOAD_SYMBOL_TYPE(block_texture_info_count, size_t);

  LOAD_SYMBOL_ADDR(block_infos);
  LOAD_SYMBOL_TYPE(block_info_count, size_t);

  LOAD_SYMBOL_ADDR(item_infos);
  LOAD_SYMBOL_TYPE(item_info_count, size_t);

  LOAD_SYMBOL_ADDR(generate_blocks);
  LOAD_SYMBOL_ADDR(generate_spawn);

  return 0;

error:
  dlclose(mod->handle);
  return -1;
}

void mod_unload(struct mod *mod)
{
  dlclose(mod->handle);
}

