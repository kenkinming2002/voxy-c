#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

struct empty
{
};

struct active_chunk
{
  ivec3_t key;
  struct empty value;
};

struct voxy_chunk_manager
{
  struct active_chunk *active_chunks;
};

void voxy_chunk_manager_init(struct voxy_chunk_manager *chunk_manager);
void voxy_chunk_manager_fini(struct voxy_chunk_manager *chunk_manager);

/// Reset the list of active chunks.
///
/// This need to be called per-frame
void voxy_chunk_manager_reset_active_chunks(struct voxy_chunk_manager *chunk_manager);

#endif // CHUNK_MANAGER_H
