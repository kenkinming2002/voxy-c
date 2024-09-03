#ifndef CHUNK_GENERATE_H
#define CHUNK_GENERATE_H

#include <libcommon/math/vector.h>
#include <libcommon/math/random.h>

struct chunk;
struct chunk_generator
{
  seed_t seed;
};

void chunk_generator_init(struct chunk_generator *chunk_generator, seed_t seed);
void chunk_generator_fini(struct chunk_generator *chunk_generator);

struct chunk *chunk_generator_generate(struct chunk_generator *chunk_generator, ivec3_t position);

#endif // CHUNK_GENERATE_H
