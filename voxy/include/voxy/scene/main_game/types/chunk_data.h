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
  bool dirty;

  block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  unsigned char block_light_levels[CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH / (CHAR_BIT / 4)];

  struct entities entities;
  struct entities new_entities;
};

/// Destroy chunk data and free all internally allocated memory.
void chunk_data_destroy(struct chunk_data *chunk_data);

/// Return if chunk data is dirtied i.e. need to be written back to disk.
bool chunk_data_is_dirty(const struct chunk_data *chunk_data);

/// Get/set block id.
block_id_t chunk_data_get_block_id(const struct chunk_data *chunk_data, ivec3_t position);
void chunk_data_set_block_id(struct chunk_data *chunk_data, ivec3_t position, block_id_t id);

/// Get/set block light level.
unsigned chunk_data_get_block_light_level(const struct chunk_data *chunk_data, ivec3_t position);
void chunk_data_set_block_light_level(struct chunk_data *chunk_data, ivec3_t position, unsigned light_level);

/// Get/set block light level atomically.
void chunk_data_get_block_light_level_atomic(const struct chunk_data *chunk_data, ivec3_t position, unsigned *light_level, unsigned char *tmp);
bool chunk_data_set_block_light_level_atomic(struct chunk_data *chunk_data, ivec3_t position, unsigned *light_level, unsigned char *tmp);

/// Convenient helper to set block given its id where light level are set
/// according block info from query_block_info().
void chunk_data_set_block(struct chunk_data *chunk_data, ivec3_t position, block_id_t id);

/// Add an entity.
///
/// Same as chunk_data_add_entity() except entity is added directly and there is no
/// need to call chunk_commit_add_entities(). It is important to make sure that
/// there is no pointer to any entity in the chunk as the underlying dynamic
/// array may be resized in the process.
void chunk_data_add_entity_raw(struct chunk_data *chunk_data, struct entity entity);

/// Add an entity.
///
/// The entity is only added after call to chunk_data_commit_add_entities().
/// This is because we might be iterating over the entity dynamic array, and we
/// have to be careful not to invalidate any pointer.
void chunk_data_add_entity(struct chunk_data *chunk_data, struct entity entity);

/// Commit adding of entities.
void chunk_data_commit_add_entities(struct chunk_data *chunk_data);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_DATA_H
