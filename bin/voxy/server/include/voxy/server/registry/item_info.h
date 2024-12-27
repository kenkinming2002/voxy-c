#ifndef VOXY_SERVER_REGISTRY_ITEM_INFO_H
#define VOXY_SERVER_REGISTRY_ITEM_INFO_H

struct voxy_entity;
struct voxy_item_stack;
struct voxy_context;

struct voxy_item_info
{
  const char *mod;
  const char *name;

  void(*on_use)(struct voxy_entity *entity, struct voxy_item_stack *item_stack, const struct voxy_context *context);
};

#endif // VOXY_SERVER_REGISTRY_ITEM_INFO_H
