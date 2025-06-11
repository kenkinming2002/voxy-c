#ifndef CHUNK_SEED_H
#define CHUNK_SEED_H

#include <libmath/random.h>

void world_init(const char *directory);

const char *world_get_directory(void);
seed_t world_get_seed(void);

#endif // CHUNK_SEED_H
