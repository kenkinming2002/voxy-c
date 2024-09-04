#ifndef MOD_MOD_H
#define MOD_MOD_H

#include <voxy/client/context.h>

typedef void *(*mod_create_instance_t)(const struct voxy_context *context);
typedef void(*mod_destroy_instance_t)(const struct voxy_context *context, void *instance);

struct mod
{
  void *handle;

  mod_create_instance_t create_instance;
  mod_destroy_instance_t destroy_instance;

  void *instance;
};

int mod_load(struct mod *mod, const char *file, const struct voxy_context *context);
void mod_unload(struct mod *mod, const struct voxy_context *context);

#endif // MOD_MOD_H
