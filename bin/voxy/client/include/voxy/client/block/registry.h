#ifndef VOXY_CLIENT_BLOCK_REGISTRY_H
#define VOXY_CLIENT_BLOCK_REGISTRY_H

#include <voxy/client/export.h>

#include "info.h"

typedef uint8_t voxy_block_id_t;
struct voxy_block_registry;

VOXY_CLIENT_EXPORT voxy_block_id_t voxy_block_registry_register_block(struct voxy_block_registry *registry, struct voxy_block_info block_info);
VOXY_CLIENT_EXPORT struct voxy_block_info voxy_block_registry_query_block(struct voxy_block_registry *registry, voxy_block_id_t id);


#endif // VOXY_CLIENT_BLOCK_REGISTRY_H
