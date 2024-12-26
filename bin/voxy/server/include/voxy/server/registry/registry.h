#ifndef VOXY_SERVER_REGISTRY_NAME_H
#define VOXY_SERVER_REGISTRY_NAME_H

#include "name_info.h"

#include <voxy/server/export.h>
#include <stdint.h>

typedef uint8_t voxy_name_id_t;
struct voxy_name_registry;

VOXY_SERVER_EXPORT voxy_name_id_t voxy_name_registry_register_name(struct voxy_name_registry *registry, struct voxy_name_info name_info);
VOXY_SERVER_EXPORT struct voxy_name_info voxy_name_registry_query_name(struct voxy_name_registry *registry, voxy_name_id_t id);

#endif // VOXY_SERVER_NAME_REGISTRY_H
