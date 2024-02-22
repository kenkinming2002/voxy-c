#include <voxy/main_game/mod.h>

#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>

void mod_load(const char *filepath)
{
  void *dl = dlopen(filepath, RTLD_LAZY);
  if(!dl)
  {
    fprintf(stderr, "WARNING: Failed to load mod from %s.\n", filepath);
    return;
  }

  void(*mod_init)(void) = dlsym(dl, "mod_init");
  if(mod_init)
    mod_init();
}

