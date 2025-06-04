#ifndef VOXY_SERVER_REGISTRY_NAME_H
#define VOXY_SERVER_REGISTRY_NAME_H

#include "name_info.h"

#include <voxy/types.h>
#include <voxy/server/export.h>

VOXY_SERVER_EXPORT voxy_name_id_t voxy_register_name(struct voxy_name_info name_info);
VOXY_SERVER_EXPORT struct voxy_name_info voxy_query_name(voxy_name_id_t id);
VOXY_SERVER_EXPORT const struct voxy_name_info *voxy_query_name_all(void);

#endif // VOXY_SERVER_NAME_REGISTRY_H
