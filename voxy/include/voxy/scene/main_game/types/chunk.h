#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H

#include <voxy/scene/main_game/config.h>

#include <voxy/scene/main_game/types/chunk_data.h>
#include <voxy/scene/main_game/types/chunk_render_info.h>

#include <voxy/math/vector.h>

#include <stdatomic.h>
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

  /// Pointer to chunk data.
  ///
  /// If this is NULL, we are either not yet generated or not loaded.
  struct chunk_data *data;

  /// Busy flag.
  ///
  /// Indicate if the chunk is currently being generated.
  bool busy : 1;

  /// Atomic pointer to new data.
  ///
  /// This is how we can obtain newly generated chunk data.
  struct chunk_data *_Atomic new_data;

  /// If we are in the singly-linked list of invalidated chunks.
  bool _Atomic mesh_invalidated;

  /// Render info for the chunk.
  ///
  /// If this is NULL, there is no render info for the current chunk probably
  /// because we are not inside the view distance.
  struct chunk_render_info *render_info;
};

/// Return if chunk is dirty.
bool chunk_is_dirty(const struct chunk *chunk);

/// Invalidate mesh at position so that any necessary remeshing is done. This
/// also takes care of also invalidating mesh of adjacent chunk if necessary.
void chunk_invalidate_mesh_at(struct chunk *chunk, ivec3_t position);

/// Get/set block id.
///
/// The *_ex version will traverse to neighbour chunk recursively. If the
/// neighbour chunk have not been generated, BLOCK_NONE is returned.
block_id_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position);
block_id_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position);
void chunk_set_block_id(struct chunk *chunk, ivec3_t position, block_id_t id);

/// Get/set block light level.
///
/// The *_ex version will traverse to neighbour chunk recursively. If the
/// neighbour chunk have not been generated, (unsigned)-1 is returned.
unsigned chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position);
unsigned chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position);
void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, unsigned light_level);

/// Get/set block light level atomically.
void chunk_get_block_light_level_atomic(const struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp);
bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp);

/// Convenient helper to set block given its id where light level are set
/// according block info from query_block_info(). This also take care of
/// triggering lighting update if necessary.
void chunk_set_block(struct chunk *chunk, ivec3_t position, block_id_t id);

/// Add an entity.
///
/// Same as chunk_add_entity() except entity is added directly and there is no
/// need to call chunk_commit_add_entities(). It is important to make sure that
/// there is no pointer to any entity in the chunk as the underlying dynamic
/// array may be resized in the process.
void chunk_add_entity_raw(struct chunk *chunk_data, struct entity entity);

/// Add an entity.
void chunk_add_entity(struct chunk *chunk_data, struct entity entity);

/// Commit adding of entities.
void chunk_commit_add_entities(struct chunk *chunk);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
