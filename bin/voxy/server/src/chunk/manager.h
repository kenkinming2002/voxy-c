#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

#include <stb_ds.h>
#include <empty.h>

struct active_chunk
{
  ivec3_t key;
  struct empty value;
};

extern struct active_chunk *active_chunks;

/// Reset the list of active chunks.
///
/// This need to be called per-frame
void voxy_reset_active_chunks(void);

#endif // CHUNK_MANAGER_H
