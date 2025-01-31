#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

#include "entity.h"

#include <sqlite3.h>

/// Entity database.
///
/// The original implmenetation is based on storing each entity in a separate
/// file and (ab)using filesystem directory structure as a indexing mechanism.
/// While it is "simple" to implement and requires minimal external dependency,
/// this necessitates the creation of lot of extremely small files, which most
/// filesystem, notably ext4, are not designed to deal with. In particular, most
/// filesystems allocate space on disk for file in the granularity of
/// blocks/sectors which may be something like 4KiB. This leads to both huge
/// space wastage and worse performance, since we can only read/write to disk a
/// sector at a time, but must of the space in a sector is unused.
///
/// Hence, we use sqlite database which should be able to pack data much more
/// efficiently.
struct voxy_entity_database
{
  sqlite3 *conn;
};

int voxy_entity_database_init(struct voxy_entity_database *database, const char *world_directory);
void voxy_entity_database_fini(struct voxy_entity_database *database);

/// Begin and end transaction.
///
/// Hopefully, this improves performance.
int voxy_entity_database_begin_transaction(struct voxy_entity_database *database);
int voxy_entity_database_end_transaction(struct voxy_entity_database *database);

/// Create an entity in database.
///
/// This should be called whenever a new entity is spawned. This is responsible
/// for populating entity->db_id.
///
/// Return non-zero on error;
int voxy_entity_database_create(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, struct voxy_entity *entity);

/// Destroy an entity in database.
///
/// This should be called whenever an entity is about to be despawned. This
/// differs from if the entity is simply unloaded in which case
/// voxy_entity_database_commit() should be called instead.
///
/// Return non-zero on error;
int voxy_entity_database_destroy(struct voxy_entity_database *database, struct voxy_entity *entity);

/// Save an entity to database.
///
/// Return non-zero on error;
int voxy_entity_database_save(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, const struct voxy_entity *entity);

/// Load an entity from database.
///
/// Return non-zero on error;
int voxy_entity_database_load(struct voxy_entity_database *database, struct voxy_entity_registry *entity_registry, struct voxy_entity *entity);

/// Uncommit entity in database.
///
/// This should be called whenever an entity is loaded. There is no need to
/// called this function if a new entity is spawned. Call
/// voxy_entity_database_create() instead.
///
/// Return non-zero on error;
int voxy_entity_database_uncommit(struct voxy_entity_database *database, int64_t db_id);

/// Commit entity in database.
///
/// This should be called whenever an entity is unloaded. There is no need to
/// called this function if a new entity is despawned. Call
/// voxy_entity_database_destroy() instead.
///
/// Return non-zero on error;
int voxy_entity_database_commit(struct voxy_entity_database *database, int64_t db_id, ivec3_t chunk_position);

DYNAMIC_ARRAY_DEFINE(db_ids, int64_t);

/// Load all active entities in database.
///
/// This should be called after the game restart in case of arupt server
/// shutdown, in which case there some of the entities maybe stuck in an active
/// state.
///
/// Return non-zero on error;
int voxy_entity_database_load_active(struct voxy_entity_database *database, struct db_ids *db_ids);

/// Load all inactive entities in database at specified chunk position.
///
/// This should be called whenever a chunk is loaded.
///
/// Return non-zero on error;
int voxy_entity_database_load_inactive(struct voxy_entity_database *database, ivec3_t chunk_position, struct db_ids *db_ids);

#endif // ENTITY_DATABASE_H
