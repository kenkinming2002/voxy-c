#ifndef VOXY_CLIENT_REGISTRY_NAME_H
#define VOXY_CLIENT_REGISTRY_NAME_H

#include "name_info.h"

#include <voxy/client/export.h>
#include <stdint.h>

typedef uint8_t voxy_name_id_t;

VOXY_CLIENT_EXPORT voxy_name_id_t voxy_register_name(struct voxy_name_info name_info);
VOXY_CLIENT_EXPORT struct voxy_name_info voxy_query_name(voxy_name_id_t id);
VOXY_CLIENT_EXPORT const struct voxy_name_info *voxy_query_name_all(void);

#endif // VOXY_CLIENT_NAME_REGISTRY_H
