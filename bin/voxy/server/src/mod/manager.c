#include "manager.h"

void mod_manager_init(struct mod_manager *mod_manager)
{
  DYNAMIC_ARRAY_INIT(mod_manager->mods);
}

void mod_manager_fini(struct mod_manager *mod_manager, struct voxy_context *context)
{
  for(size_t i=0; i<mod_manager->mods.item_count; ++i)
    mod_unload(&mod_manager->mods.items[i], context);

  DYNAMIC_ARRAY_CLEAR(mod_manager->mods);
}

int mod_manager_load(struct mod_manager *mod_manager, const char *file, struct voxy_context *context)
{
  struct mod mod;
  if(mod_load(&mod, file, context) != 0)
    return -1;

  DYNAMIC_ARRAY_APPEND(mod_manager->mods, mod);
  return 0;
}
