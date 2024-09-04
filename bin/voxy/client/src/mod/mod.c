#include "mod.h"

#include <libcommon/core/log.h>

#include <dlfcn.h>

int mod_load(struct mod *mod, const char *file, const struct voxy_context *context)
{
  if(!(mod->handle = dlopen(file, RTLD_LAZY | RTLD_LOCAL)))
  {
    LOG_ERROR("Failed to load mod: %s", dlerror());
    goto error0;
  }

  if(!(mod->create_instance = dlsym(mod->handle, "mod_create_instance")))
  {
    LOG_ERROR("Missing mod_create_instance symbol from mod: %s", dlerror());
    goto error1;
  }

  if(!(mod->destroy_instance = dlsym(mod->handle, "mod_destroy_instance")))
  {
    LOG_ERROR("Missing mod_destroy_instance symbol from mod: %s", dlerror());
    goto error1;
  }

  mod->instance = mod->create_instance(context);
  return 0;

error1:
  dlclose(mod->handle);
error0:
  return -1;
}

void mod_unload(struct mod *mod, const struct voxy_context *context)
{
  mod->destroy_instance(context, mod->instance);
}
