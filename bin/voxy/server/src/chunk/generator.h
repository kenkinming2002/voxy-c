#ifndef CHUNK_GENERATOR_H
#define CHUNK_GENERATOR_H

#include <voxy/server/chunk/generator.h>

struct voxy_chunk;
struct voxy_chunk_generator
{
  seed_t seed;
  voxy_generate_chunk_t generate_chunk;
};

void voxy_chunk_generator_init(struct voxy_chunk_generator *chunk_generator, seed_t seed);
void voxy_chunk_generator_fini(struct voxy_chunk_generator *chunk_generator);

struct voxy_chunk *voxy_chunk_generator_generate(struct voxy_chunk_generator *chunk_generator, ivec3_t position, const struct voxy_context *context);

#endif // CHUNK_GENERATOR_H
