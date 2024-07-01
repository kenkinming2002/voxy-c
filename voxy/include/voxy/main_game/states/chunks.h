#ifndef VOXY_MAIN_GAME_STATES_CHUNKS_H
#define VOXY_MAIN_GAME_STATES_CHUNKS_H

#include <voxy/main_game/types/chunk.h>
#include <voxy/main_game/types/chunk_hash_table.h>

/// The chunk system, which is at the heart of our voxel game. (Unless you go
/// with the route of octree).
///
/// The data structure we use is a hash table with chunk position as the key.
/// However, hash table lookup even though they are techically O(1) can be
/// expensive especially if we want a good hashing function. This is why we
/// stores pointers to neighbouring chunks in the 6 axial directions, so that
/// accessing neighoburing chunks and hence blocks can be done simply by chasing
/// pointers. This may or may not change in the future.

/// A macro helpers to iterate over all chunks. Normally in other language, this
/// would be abstracted using iterators or callbacks, but we are programming in
/// C. Iterators will still leak implementation details and callbacks are not
/// really ergnomic in C because we do not have lambdas.
extern struct chunk_hash_table world_chunks;
#define world_for_each_chunk(it)                                               \
  for (size_t i = 0; i < world_chunks.bucket_count; ++i)                             \
    for (struct chunk *it = world_chunks.buckets[i].head; it; it = it->next)

/// Get a pointer to a chunk at position, which is specified by chunk position.
///
/// This will *create* the chunk if it does not exist yet but none of its
/// internal states would be initialized. This may change in the future.
struct chunk *world_get_chunk(ivec3_t position);

/// Get a pointer to a block at position, which is specified in global
/// coordinate.
///
/// Unlike chunk_data_get_block(), this function may fail if data field of
/// target chunk is NULL, which signifies that the chunk is either not generated
/// yet or not loaded. NULL is returned in such a case.
///
/// If you have an exising pointer to the target chunk or a nearby chunk,
/// consider using chunk_get_block() instead.
///
/// If you intend on modifying the block, make sure to send invaldiation event
/// by calling world_invalidate_block().
struct block *world_get_block(ivec3_t position);

/// Invalidate the block at position.
///
/// This need to be called if block at position is changed.
void world_invalidate_block(ivec3_t position);

/// Add an entity.
///
/// Unlike chunk_add_entity(), the correct chunk to add the entity to would be
/// located based-off of entity position.
///
/// Like chunk_add_entity(), this function may fail if data field of target
/// chunk is NULL, which signifies that the chunk is either not generated yet or
/// not loaded. NULL is returned in such a case.
///
/// Like chunk_add_entity(), the returned pointer is valid until another entity
/// is added which may cause reallocation in the underlying dynamic array.
struct entity *world_add_entity(struct entity entity);

#endif // VOXY_MAIN_GAME_STATES_CHUNKS_H
