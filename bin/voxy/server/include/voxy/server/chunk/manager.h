#ifndef VOXY_SERVER_CHUNK_MANAGER_H
#define VOXY_SERVER_CHUNK_MANAGER_H

#include <voxy/server/export.h>

#include <libcommon/math/vector.h>

struct chunk_manager;

VOXY_SERVER_EXPORT void chunk_manager_add_active_chunk(struct chunk_manager *chunk_manager, ivec3_t position);

#endif // VOXY_SERVER_CHUNK_MANAGER_H
