#ifndef VOXY_SCENE_MAIN_GAME_STATES_CHUNKS_H
#define VOXY_SCENE_MAIN_GAME_STATES_CHUNKS_H

#include <voxy/scene/main_game/types/chunk.h>
#include <voxy/scene/main_game/types/chunk_hash_table.h>

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

/// Coordinate conversions.
///
/// FIXME: This probably should be moved to some other module.
ivec3_t get_chunk_position(ivec3_t position);
ivec3_t global_position_to_local_position(ivec3_t position);
ivec3_t local_position_to_global_position(ivec3_t position, ivec3_t chunk_position);

/// Get a pointer to a chunk at position, which is specified by chunk position.
///
/// This will *create* the chunk if it does not exist yet but none of its
/// internal states would be initialized. This may change in the future.
struct chunk *world_get_chunk(ivec3_t position);

/// Invalidate mesh at position so that any necessary remeshing is done.
void world_invalidate_mesh_at(ivec3_t position);

/// Getters.
///
/// This take care of splitting position to locate the correct chunk and
/// position within said chunk.
///
/// This may fail if the chunk is not yet generated in which case BLOCK_NONE or
/// (unsigned)-1 is returned.
///
/// Technically, it can be more efficient to return block id, ether and light
/// level at once as it avoid repeated hash table lookup internally but I don't
/// cares :). The point is, cares need to be taken if they are to be used in
/// performance-critical code.
block_id_t world_get_block_id(ivec3_t position);
unsigned world_get_block_ether(ivec3_t position);
unsigned world_get_block_light_level(ivec3_t position);

/// Set a block at position by its id.
///
/// This takes of sending invalidation events to all relevant systems, and
/// calling all relevant callbacks.
void world_set_block(ivec3_t position, block_id_t id, struct entity *entity);

/// Add an entity.
///
/// Same as world_add_entity() except entity is added directly and there is no
/// need to call chunk_commit_add_entities(). It is important to make sure that
/// there is no pointer to any entity in the chunk as the underlying dynamic
/// array may be resized in the process.
bool world_add_entity_raw(struct entity entity);

/// Add an entity.
///
/// This take care of splitting entity position to locate the correct chunk to
/// add the entity.
///
/// This may fail if the chunk is not yet generated in which case false is
/// returned.
bool world_add_entity(struct entity entity);

#endif // VOXY_SCENE_MAIN_GAME_STATES_CHUNKS_H
