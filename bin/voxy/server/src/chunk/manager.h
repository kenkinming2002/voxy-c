#ifndef CHUNK_MANAGER_H
#define CHUNK_MANAGER_H

#include <voxy/server/chunk/manager.h>

#include <stdbool.h>

/// Reset the list of chunk regions.
///
/// This need to be called per-frame
void voxy_reset_chunk_regions(void);

/// Iterate through the list of major chunks specified by current list of chunk
/// regions.
void iterate_major_chunk(bool(*iterate)(ivec3_t chunk_position, void *data), void *data);

/// Check if the chunk at position is in the list of minor chunk.
bool is_minor_chunk(ivec3_t chunk_position);

#endif // CHUNK_MANAGER_H
