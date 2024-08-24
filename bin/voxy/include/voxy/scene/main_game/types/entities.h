#ifndef VOXY_SCENE_MAIN_GAME_TYPES_ENTITIES_H
#define VOXY_SCENE_MAIN_GAME_TYPES_ENTITIES_H

#include <libcommon/utils/dynamic_array.h>

#include "entity.h"

DYNAMIC_ARRAY_DEFINE(entities, struct entity);

void entities_init(struct entities *entities);
void entities_fini(struct entities *entities);
void entities_append(struct entities *entities, struct entity entity);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_ENTITIES_H
