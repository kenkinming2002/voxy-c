#ifndef VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
#define VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H

#include <voxy/scene/main_game/config.h>

#include <voxy/scene/main_game/types/registry.h>

#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/entities.h>

#include <voxy/scene/main_game/types/block_data.h>
#include <voxy/scene/main_game/types/block_datas.h>

#include <libcommon/math/vector.h>

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

  /// Remesh flag.
  ///
  /// If chunk need to be remeshed.
  ///
  /// The only reason it is atomic is because of our lighting system.
  atomic_bool remesh;

  /// Dirty flag .
  ///
  /// If chunk need to be written back onto the disk.
  ///
  /// The only reason it is atomic is because of our lighting system.
  atomic_bool dirty;

  /// Last time chunk data is written back onto the disk.
  ///
  /// This allows us to throttle write back to disk.
  float last_save_time;

  /// Block.
  block_id_t block_ids[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];
  unsigned char block_light_levels[CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH / (CHAR_BIT / 4)];

  /// Block datas
  struct block_datas block_datas;

  /// Entities.
  struct entities entities;
  struct entities new_entities;
};

/// Destroy chunk.
void chunk_destroy(struct chunk *chunk);

/// Return if chunk is dirty.
bool chunk_is_dirty(const struct chunk *chunk);

/// Return if chunk should be written back on to the disk i.e.
///  - It is dirty.
///  - There has been sufficient time elapsed since last write back.
bool chunk_should_save(const struct chunk *chunk);

/// Invalidate mesh so that any necessary remeshing is done. This also takes
/// care of also invalidating mesh of adjacent chunk if necessary.
///
/// The *_at() variant take an additional position indicating which blocks have
/// been changed, so that only neighbouring chunk adjacent to that block has its
/// mesh invalidated.
void chunk_invalidate_mesh(struct chunk *chunk);
void chunk_invalidate_mesh_at(struct chunk *chunk, ivec3_t position);

/// Invalidate data so that disk write back may happen.
///
/// The *_at() variant take an additional position indicating which blocks have
/// been changed. This is for consistency with the API for mesh invalidation but
/// otherwise serves no purpose.
void chunk_invalidate_data(struct chunk *chunk);
void chunk_invalidate_data_at(struct chunk *chunk, ivec3_t position);

/// Get/set block id.
///
/// The *_ex version will traverse to neighbour chunk recursively. If the
/// neighbour chunk have not been generated, BLOCK_NONE is returned.
block_id_t chunk_get_block_id(const struct chunk *chunk, ivec3_t position);
block_id_t chunk_get_block_id_ex(const struct chunk *chunk, ivec3_t position);
void chunk_set_block_id_raw(struct chunk *chunk, ivec3_t position, block_id_t id);
void chunk_set_block_id(struct chunk *chunk, ivec3_t position, block_id_t id);

/// Get/set block light level.
///
/// The *_ex version will traverse to neighbour chunk recursively. If the
/// neighbour chunk have not been generated, (unsigned)-1 is returned.
unsigned chunk_get_block_light_level(const struct chunk *chunk, ivec3_t position);
unsigned chunk_get_block_light_level_ex(const struct chunk *chunk, ivec3_t position);
void chunk_set_block_light_level_raw(struct chunk *chunk, ivec3_t position, unsigned light_level);
void chunk_set_block_light_level(struct chunk *chunk, ivec3_t position, unsigned light_level);

/// Get/set block light level atomically.
void chunk_get_block_light_level_atomic(const struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp);
bool chunk_set_block_light_level_atomic(struct chunk *chunk, ivec3_t position, unsigned *light_level, unsigned char *tmp);

/// Convenient helper to set block given its id where light level are set
/// according block info from query_block_info().
void chunk_set_block_raw(struct chunk *chunk, ivec3_t position, block_id_t id);

/// Convenient helper to set block given its id where light level are set
/// according block info from query_block_info(). This also take care of
/// triggering lighting update if necessary.
void chunk_set_block(struct chunk *chunk, ivec3_t position, block_id_t id);

/// Add/del/get block data.
void chunk_add_block_data(struct chunk *chunk, ivec3_t position, void *data);
void *chunk_del_block_data(struct chunk *chunk, ivec3_t position);
void *chunk_get_block_data(const struct chunk *chunk, ivec3_t position);

/// Commit removal of entities.
///
/// An entity is removed only if its remove field is set to true.
void chunk_commit_remove_entities(struct chunk *chunk);

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

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#endif // VOXY_SCENE_MAIN_GAME_TYPES_CHUNK_H
