#ifndef VOXY_SERVER_ITEM_CONTAINER_H
#define VOXY_SERVER_ITEM_CONTAINER_H

#include <voxy/server/export.h>
#include <voxy/server/item/stack.h>

struct voxy_item_container
{
  uint16_t width, height;
  struct voxy_item_stack *slots;
};

#endif // VOXY_SERVER_ITEM_CONTAINER_H
