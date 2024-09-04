#ifndef VOXY_SERVER_CHUNK_MANAGER_H
#define VOXY_SERVER_CHUNK_MANAGER_H

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>

struct voxy_chunk_manager;

VOXY_SERVER_EXPORT void voxy_chunk_manager_add_active_chunk(struct voxy_chunk_manager *chunk_manager, ivec3_t position);

#endif // VOXY_SERVER_CHUNK_MANAGER_H
