#ifndef CHUNK_ENTITY_DATABASE_H
#define CHUNK_ENTITY_DATABASE_H

#include "entity.h"

#include <sqlite3.h>

void voxy_entity_database_init(const char *world_directory);

/// Begin and end transaction.
///
/// Hopefully, this improves performance.
int voxy_entity_database_begin_transaction(void);
int voxy_entity_database_end_transaction(void);

/// Create an entity in database.
///
/// This should be called whenever a new entity is spawned. This is responsible
/// for populating entity->db_id.
///
/// Return non-zero on error;
int voxy_entity_database_create(struct voxy_entity *entity);

/// Destroy an entity in database.
///
/// This should be called whenever an entity is about to be despawned. This
/// differs from if the entity is simply unloaded in which case
/// voxy_entity_database_commit() should be called instead.
///
/// Return non-zero on error;
int voxy_entity_database_destroy(struct voxy_entity *entity);

/// Save an entity to database.
///
/// Return non-zero on error;
int voxy_entity_database_save(const struct voxy_entity *entity);

/// Load an entity from database.
///
/// Return non-zero on error;
int voxy_entity_database_load(struct voxy_entity *entity);

/// Uncommit entity in database.
///
/// This should be called whenever an entity is loaded. There is no need to
/// called this function if a new entity is spawned. Call
/// voxy_entity_database_create() instead.
///
/// Return non-zero on error;
int voxy_entity_database_uncommit(int64_t db_id);

/// Commit entity in database.
///
/// This should be called whenever an entity is unloaded. There is no need to
/// called this function if a new entity is despawned. Call
/// voxy_entity_database_destroy() instead.
///
/// Return non-zero on error;
int voxy_entity_database_commit(int64_t db_id, ivec3_t chunk_position);

/// Load all active entities in database.
///
/// This should be called after the game restart in case of arupt server
/// shutdown, in which case there some of the entities maybe stuck in an active
/// state.
///
/// Return non-zero on error;
int voxy_entity_database_load_active(int64_t **db_ids);

/// Load all inactive entities in database at specified chunk position.
///
/// This should be called whenever a chunk is loaded.
///
/// Return non-zero on error;
int voxy_entity_database_load_inactive(ivec3_t chunk_position, int64_t **db_ids);

#endif // CHUNK_ENTITY_DATABASE_H
