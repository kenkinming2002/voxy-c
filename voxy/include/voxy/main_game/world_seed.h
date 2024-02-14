#ifndef MAIN_GAME_WORLD_SEED_H
#define MAIN_GAME_WORLD_SEED_H

#include <voxy/math/random.h>

void world_seed_set(seed_t seed);
seed_t world_seed_get(void);

void world_seed_generate(void);

#endif // MAIN_GAME_WORLD_SEED_H
