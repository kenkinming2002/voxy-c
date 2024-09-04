#ifndef VOXY_SERVER_BLOCK_REGISTRY_H
#define VOXY_SERVER_BLOCK_REGISTRY_H

#include "info.h"

#include <voxy/server/export.h>

#include <stdint.h>

typedef uint8_t block_id_t;
struct voxy_block_registry;

VOXY_SERVER_EXPORT block_id_t voxy_block_registry_register_block(struct voxy_block_registry *registry, struct voxy_block_info block_info);
VOXY_SERVER_EXPORT struct voxy_block_info voxy_block_registry_query_block(struct voxy_block_registry *registry, block_id_t id);

#endif // VOXY_SERVER_BLOCK_REGISTRY_H
