#ifndef VOXY_SCENE_MAIN_GAME_STATES_SEED_H
#define VOXY_SCENE_MAIN_GAME_STATES_SEED_H

#include <voxy/math/random.h>

void save_world_seed(void);
void load_world_seed(void);

void world_seed_set(seed_t seed);
seed_t world_seed_get(void);

void world_seed_generate(void);

#endif // VOXY_SCENE_MAIN_GAME_STATES_SEED_H
