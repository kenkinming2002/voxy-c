#ifndef VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H
#define VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H

#include <voxy/math/vector.h>

struct chunk;

/// Enqueue a light update at position indicating that a light source has been
/// created.
void enqueue_light_create_update(ivec3_t position);

/// Enqueue a light update at position indicating that a light source has been
/// destroyed. Given are the original light level and ether.
void enqueue_light_destroy_update(ivec3_t position, unsigned light_level, unsigned ether);

/// Raw API to enqueue_light_create_update().
void enqueue_light_create_update_raw(struct chunk *chunk, unsigned x, unsigned y, unsigned z);

/// Raw API to enqueue_light_destroy_update().
void enqueue_light_destroy_update_raw(struct chunk *chunk, unsigned x, unsigned y, unsigned z, unsigned light_level, unsigned ether);

/// Perform ligting update. This is called at the end of each frame.
void update_light(void);

#endif // VOXY_SCENE_MAIN_GAME_UPDATE_LIGHT_H
