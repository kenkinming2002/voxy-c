#include "mod.h"

#include <libcore/log.h>

#include <stdlib.h>
#include <dlfcn.h>

void mod_load(const char *file, const struct voxy_context *context)
{
  void *handle = dlopen(file, RTLD_LAZY | RTLD_LOCAL);

  int(*mod_init)(const struct voxy_context *context) = dlsym(handle, "mod_init");
  if(!mod_init)
  {
    LOG_ERROR("Missing mod_init symbol from mod: %s", dlerror());
    exit(EXIT_FAILURE);
  }

  int result = mod_init(context);
  if(result != 0)
  {
    LOG_ERROR("Failed to initialize mod: %d", result);
    exit(EXIT_FAILURE);
  }
}

