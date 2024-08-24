#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H

#include <libcommon/math/vector.h>

struct chunk;

/// Raw API to enqueue_light_create_update().
void enqueue_light_create_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z);

/// Raw API to enqueue_light_destroy_update().
void enqueue_light_destroy_update(struct chunk *chunk, unsigned x, unsigned y, unsigned z, unsigned light_level);

/// Perform ligting update. This is called at the end of each frame.
void update_light(void);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H
