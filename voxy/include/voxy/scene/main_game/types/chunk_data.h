#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>

#include <voxy/dynamic_array.h>

#include <stdatomic.h>

DYNAMIC_ARRAY_DEFINE(entities, struct entity);

/// Chunk data.
///
/// This store persistent data local to a chunk such as blocks or entities.
struct chunk_data
{
  /// Dirty flag .
  ///
  /// If chunk data need to be written back onto the disk.
  bool dirty : 1;

  struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

  struct entities entities;
  struct entities new_entities;
};

/// Destroy chunk data and free all internally allocated memory.
void chunk_data_destroy(struct chunk_data *chunk_data);

/// Get a pointer to a block at position, which is specified in chunk local
/// coordinate in the range [0, CHUNK_WIDTH)x[0, CHUNK_WIDTH)x[0, CHUNK_WIDTH).
///
/// If you would also like to access block in neighbouring chunks, consider
/// using chunk_get_block().
struct block *chunk_data_get_block(struct chunk_data *chunk_data, ivec3_t position);

/// Add an entity.
///
/// The entity is only added after call to chunk_data_commit_add_entities().
/// This is because we might be iterating over the entity dynamic array, and we
/// have to be careful not to invalidate any pointer.
///
/// This always succeeds unless you run out of memory in which case you have
/// bigger problem.
///
/// The returned pointer is valid until another entity is added which may cause
/// reallocation in the underlying dynamic array.
void chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity);

/// Add an entity.
///
/// Same as chunk_data_add_entity() except entity is added directly and there is no
/// need to call chunk_commit_add_entities(). It is important to make sure that
/// there is no pointer to any entity in the chunk as the underlying dynamic
/// array may be resized in the process.
void chunk_data_add_entity_raw(struct chunk_data *chunk_data, struct entity entity);

/// Commit adding of entities.
///
/// See chunk_data_add_entity() for more details.
void chunk_data_commit_add_entities(struct chunk_data *chunk_data);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H
