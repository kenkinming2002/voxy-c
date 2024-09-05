#ifndef VOXY_SERVER_ENTITY_REGISTRY_H
#define VOXY_SERVER_ENTITY_REGISTRY_H

#include "info.h"

#include <voxy/server/export.h>

#include <stdint.h>

typedef uint8_t voxy_entity_id_t;
struct voxy_entity_registry;

VOXY_SERVER_EXPORT voxy_entity_id_t voxy_entity_registry_register_entity(struct voxy_entity_registry *registry, struct voxy_entity_info entity_info);
VOXY_SERVER_EXPORT struct voxy_entity_info voxy_entity_registry_query_entity(struct voxy_entity_registry *registry, voxy_entity_id_t id);

#endif // VOXY_SERVER_ENTITY_REGISTRY_H
