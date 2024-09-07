#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include "block/registry.h"

#include <libcommon/math/vector.h>
#include <libcommon/math/random.h>

struct chunk;
struct chunk_generator
{
  seed_t seed;
};

void chunk_generator_init(struct chunk_generator *chunk_generator, seed_t seed);
void chunk_generator_fini(struct chunk_generator *chunk_generator);

struct chunk *chunk_generator_generate(struct chunk_generator *chunk_generator, ivec3_t position, struct voxy_block_registry *block_registry);

#endif // CHUNK_GENERATOR_H
