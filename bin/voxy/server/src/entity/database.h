#ifndef ENTITY_DATABASE_H
#define ENTITY_DATABASE_H

#include "entity.h"

/// Entity database.
///
/// Following is a description of the directory structure we use.
///
/// Each entity is stored in a separate file called a blob in
/// <ENTITIES_PATH>/blobs/<COOKIE> where <ENTITIES_PATH> defaults to
/// "world/entities" and <COOKIE> are randomly generated for each entity.
///
/// An entity can be in a loaded state, if it is inside a loaded chunk. In such
/// a case, a hard link to it would be created under the directory
/// <ENTITIES_PATH>/active. Otherwise, there will be hard link to it under
/// <ENTITIES_PATH>/<X>/<Y>/<Z> where <X>, <Y>, <Z> are components for the
/// position of the chunk that the entity is in.
///
/// On startup after an arupt shutdown, entities under <ENTITIES_PATH>/active
/// will be loaded and re-linked under <ENTITIES_PATH>/<X>/<Y>/<Z> depending on
/// which chunk each entity is in.

/// Create an entity in the database.
///
/// This should be called when an entity is spawned. A blob will be allocated
/// for the entity and the entity will be put in loaded state i.e. there will be
/// a hard link to it under <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_create_entity(struct voxy_entity *entity);

/// Destroy an entity in the database.
///
/// This should be called when an entity is despawned. This assumes that the
/// entity is in a loaded state i.e. there is a hard link to its blob under
/// <ENTITIES_PATH>/active. The entity blob will be deallocated and the hard
/// link to it will be removed.
///
/// A blob will be allocated
/// for the entity and the entity will be put in loaded state i.e. there will be
/// a hard link to it under <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_destroy_entity(const struct voxy_entity *entity);

/// Update an entity in the database.
///
/// Return non-zero value on failure.
int voxy_entity_database_update_entity(const struct voxy_entity *entity);

/// Commit an entity in the database.
///
/// This should be called when an entity is to be unloaded. This move the hard
/// link to the entity under <ENTITIES_PATH>/active to
/// <ENTITIES_PATH>/<X>/<Y>/<Z>.
///
/// Return non-zero value on failure.
int voxy_entity_database_commit_entity(const struct voxy_entity *entity);

/// Commit an entity in the database.
///
/// This should be called after an entity is loaded. This move the hard link to
/// the entity under <ENTITIES_PATH>/<X>/<Y>/<Z> to <ENTITIES_PATH>/active.
///
/// Return non-zero value on failure.
int voxy_entity_database_uncommit_entity(const struct voxy_entity *entity, ivec3_t chunk_position);

/// Load active entities.
///
/// Return non-zero value on failure.
int voxy_entity_database_load_active_entities(struct voxy_entities *entities);

/// Load inactive entities in given chunk position.
///
/// Return non-zero value on failure.
int voxy_entity_database_load_inactive_entities(ivec3_t chunk_position, struct voxy_entities *entities);

#endif // ENTITY_DATABASE_H
