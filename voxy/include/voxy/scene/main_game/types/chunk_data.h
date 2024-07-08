#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/types/block.h>
#include <voxy/scene/main_game/types/entity.h>

/// Chunk data.
///
/// This store persistent data local to a chunk such as blocks or entities.
struct chunk_data
{
  struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

  struct entity *entities;
  size_t         entity_count;
  size_t         entity_capacity;

  struct entity *new_entities;
  size_t         new_entity_count;
  size_t         new_entity_capacity;
};

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

/// Commit adding of entities.
///
/// See chunk_data_add_entity() for more details.
void chunk_data_commit_add_entities(struct chunk_data *chunk_data);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H
