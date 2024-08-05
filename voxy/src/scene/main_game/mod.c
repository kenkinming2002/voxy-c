#include <voxy/scene/main_game/mod.h>

#include <libcommon/core/log.h>

#include <dlfcn.h>
#include <stdlib.h>

void(*mod_init)(void);
void (*mod_enter)(void);
void (*mod_leave)(void);
void(*mod_update)(void);

void mod_load(const char *filepath)
{
  void *dl = dlopen(filepath, RTLD_LAZY);
  if(!dl)
  {
    LOG_ERROR("Failed to load mod from %s", filepath);
    exit(EXIT_FAILURE);
  }

  mod_init = dlsym(dl, "mod_init");
  mod_enter = dlsym(dl, "mod_enter");
  mod_leave = dlsym(dl, "mod_leave");
  mod_update = dlsym(dl, "mod_update");

  if(mod_init)
    mod_init();
}

