#include <voxy/scene/main_game/mod.h>

#include <voxy/core/log.h>

#include <dlfcn.h>
#include <stdlib.h>

void mod_load(const char *filepath)
{
  void *dl = dlopen(filepath, RTLD_LAZY);
  if(!dl)
  {
    LOG_ERROR("Failed to load mod from %s", filepath);
    exit(EXIT_FAILURE);
  }

  void(*mod_init)(void) = dlsym(dl, "mod_init");
  if(mod_init)
    mod_init();
}

