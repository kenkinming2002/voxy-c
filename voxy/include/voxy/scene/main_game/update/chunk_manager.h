#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_MANAGER_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_MANAGER_H

#include <libcommon/math/vector.h>

/// Reset list of active chunks.
///
/// This should be called at the beginning of each frame.
void reset_active_chunks(void);

/// Add the chunk at position to the set of active chunks.
///
/// Chunks that are active will be loaded from disk if it exist or generated
/// otherwise. Chunks that are not active will be saved to disk if it is still
/// in memory.
void activate_chunk(ivec3_t position);

/// This is what perform the heavy lifting of generating/saving/loading chunks
/// as specified by load_chunk().
void sync_active_chunks(void);

/// Flush all dirtied active chunks onto the disk regardless of whether we have
/// just save them or not. This need to be called before we exit main game or
/// else we have data loss.
void flush_active_chunks(void);

/// Save list of activated chunks.
void save_active_chunks(void);

/// Load list of activated chunks.
void load_active_chunks(void);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_CHUNK_MANAGER_H
