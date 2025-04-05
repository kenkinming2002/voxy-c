#include "manager.h"

#include <stb_ds.h>

void mod_manager_init(struct mod_manager *mod_manager)
{
  mod_manager->mods = NULL;
}

void mod_manager_fini(struct mod_manager *mod_manager, struct voxy_context *context)
{
  for(size_t i=0; i<arrlenu(mod_manager->mods); ++i)
    mod_unload(&mod_manager->mods[i], context);

  arrfree(mod_manager->mods);
}

int mod_manager_load(struct mod_manager *mod_manager, const char *file, struct voxy_context *context)
{
  struct mod mod;
  if(mod_load(&mod, file, context) != 0)
    return -1;

  arrput(mod_manager->mods, mod);
  return 0;
}
