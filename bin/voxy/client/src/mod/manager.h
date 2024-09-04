#ifndef MOD_MANAGER_H
#define MOD_MANAGER_H

#include "mod.h"

#include <libcommon/utils/dynamic_array.h>

struct mod_manager
{
  DYNAMIC_ARRAY_DEFINE(, struct mod) mods;
};

void mod_manager_init(struct mod_manager *mod_manager);
void mod_manager_fini(struct mod_manager *mod_manager, struct voxy_context *context);

int mod_manager_load(struct mod_manager *mod_manager, const char *file, struct voxy_context *context);

#endif // MOD_MANAGER_H
