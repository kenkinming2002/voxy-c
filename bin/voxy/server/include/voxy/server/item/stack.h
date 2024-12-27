#ifndef VOXY_SERVER_ITEM_STACK_H
#define VOXY_SERVER_ITEM_STACK_H

#include <voxy/server/export.h>
#include <voxy/server/registry/item.h>

struct voxy_item_stack
{
  voxy_item_id_t id;
  uint8_t count;
};

#endif // VOXY_SERVER_ITEM_STACK_H
