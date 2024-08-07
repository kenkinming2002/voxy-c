#ifndef VOXY_SCENE_MAIN_GAME_STATES_ENTITY_QUERY_H
#define VOXY_SCENE_MAIN_GAME_STATES_ENTITY_QUERY_H

#include <voxy/scene/main_game/types/entity.h>

#include <libcommon/math/aabb.h>

/// Prepare for the querying of entity. This need to be called at the beginning
/// of each frame.
void world_query_entity_begin(void);

/// Free up all the internal data structures allocated. This need to be called
/// at the end of each frame.
void world_query_entity_end(void);

/// Query for entities whose hitbox intersect with given aabb.
///
/// The returned entities pointer need to be deallocated with free(3).
void world_query_entity(aabb3_t aabb, struct entity ***entities, size_t *entity_count);

#endif // VOXY_SCENE_MAIN_GAME_STATES_ENTITY_QUERY_H
