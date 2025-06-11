#ifndef CHUNK_ENTITY_DATABASE_H
#define CHUNK_ENTITY_DATABASE_H

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
///
/// Enough shit talking and lets talk about the implementation.
///
/// Conceptually, an entity can be in one of two states - active and inactive.
/// An entity is in active state if it is currently loaded. Otherwise, it is in
/// an inactive state. In diagram form:
///
///                                         |  ^
///                                  create |  | destroy
///                                         v  |
/// ------------         unload          ----------
/// |          | ----------------------> |        | ---
/// | Inactive |                         | Active |   | update
/// |          | <---------------------- |        | <--
/// ------------     load_inactive       ----------
///                                        ^   |
///                                        |   |
///                                        |---|
///                                     load active
///                     (in case we did not unload entity before exit)

/// Initialize entity database.
///
/// This basically just open the database connection and arrange for it to be
/// closed atexit(3).
void voxy_entity_database_init(const char *world_directory);

/// Begin and end transaction.
///
/// Hopefully, this improves performance.
void voxy_entity_database_begin_transaction(void);
void voxy_entity_database_end_transaction(void);

/// Create an entity in database.
///
/// This should be called whenever a new entity is spawned. This is responsible
/// for populating entity->db_id.
void voxy_entity_database_create(struct voxy_entity *entity);

/// Update an entity in database.
void voxy_entity_database_update(const struct voxy_entity *entity);

/// Destroy an entity in database.
///
/// This should be called whenever an entity is about to be despawned.
void voxy_entity_database_destroy(struct voxy_entity *entity);

/// Unload an entity.
///
/// This converts an entity from active to inactive.
void voxy_entity_database_unload(struct voxy_entity *entity);

/// Load all active entities in database.
///
/// This should be called after the game restart in case of arupt server
/// shutdown, in which case there some of the entities maybe stuck in an active
/// state.
void voxy_entity_database_load_active(struct voxy_entity **entities);

/// Load all inactive entities in database at specified chunk position.
///
/// This should be called whenever a chunk is loaded.
void voxy_entity_database_load_inactive(ivec3_t chunk_position, struct voxy_entity **entities);

#endif // CHUNK_ENTITY_DATABASE_H
