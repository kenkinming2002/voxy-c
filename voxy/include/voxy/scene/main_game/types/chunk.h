#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H

#include <voxy/scene/main_game/config.h>

#include <voxy/scene/main_game/types/chunk_data.h>
#include <voxy/scene/main_game/types/chunk_render_info.h>

#include <voxy/math/vector.h>

#include <stddef.h>
#include <stdbool.h>

/// The chunk data structure.
struct chunk
{
  /// Key.
  ivec3_t position;

  /// Metadata for hash table.
  size_t        hash;
  struct chunk *next;

  /// Pointers to neighbouring chunks.
  struct chunk *bottom;
  struct chunk *top;
  struct chunk *back;
  struct chunk *front;
  struct chunk *left;
  struct chunk *right;

  /// If we are in the singly-linked list of invalidated chunks.
  bool mesh_invalidated : 1;
  bool light_invalidated : 1;

  /// Pointers to next chunk in the singly-linked list of chunks that have been
  /// invalidated.
  struct chunk *light_next;

  /// Pointer to chunk data.
  ///
  /// If this is NULL, we are either not yet generated or not loaded.
  struct chunk_data *data;

  /// Render info for the chunk.
  ///
  /// If this is NULL, there is no render info for the current chunk probably
  /// because we are not inside the view distance.
  struct chunk_render_info *render_info;
};

/// Get a pointer to a block at position, which is specified in chunk local
/// coordinate.
///
/// Unlike chunk_data_get_block(), position may be outside of the
/// range [0, CHUNK_WIDTH)x[0, CHUNK_WIDTH)x[0, CHUNK_WIDTH) in which case
/// neighbouring chunks would be traversed recursively to locate the correct
/// chunk.
///
/// Unlike chunk_data_get_block(), this function may fail if data field of target
/// chunk is NULL, which signifies that the chunk is either not generated yet or
/// not loaded. NULL is returned in such a case.
///
/// This may be slow if the position is far from the current chunk. In that case
/// consider using world_get_block() instead.
struct block *chunk_get_block(struct chunk *chunk, ivec3_t position);

/// Add an entity.
///
/// Unlike chunk_data_add_entity(), this function may fail if data field of
/// target chunk is NULL, which signifies that the chunk is either not generated
/// yet or not loaded. NULL is returned in such a case.
///
/// Like chunk_data_add_entity(), the returned pointer is valid until another
/// entity is added which may cause reallocation in the underlying dynamic array.
struct entity *chunk_add_entity(struct chunk *chunk_data, struct entity entity);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
