#ifndef VOXY_CLIENT_ENTITY_REGISTRY_H
#define VOXY_CLIENT_ENTITY_REGISTRY_H

#include <voxy/client/export.h>

#include "info.h"

typedef uint8_t entity_id_t;
struct voxy_entity_registry;

VOXY_CLIENT_EXPORT entity_id_t voxy_entity_registry_register_entity(struct voxy_entity_registry *registry, struct voxy_entity_info entity_info);
VOXY_CLIENT_EXPORT struct voxy_entity_info voxy_entity_registry_query_entity(struct voxy_entity_registry *registry, entity_id_t id);

#endif // VOXY_CLIENT_ENTITY_REGISTRY_H
