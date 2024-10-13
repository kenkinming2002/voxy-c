#include "generator.h"

#include "chunk.h"

void voxy_chunk_generator_init(struct voxy_chunk_generator *chunk_generator, seed_t seed)
{
  chunk_generator->seed = seed;
  chunk_generator->generate_chunk = NULL;
}

void voxy_chunk_generator_fini(struct voxy_chunk_generator *chunk_generator)
{
  (void)chunk_generator;
}

void voxy_chunk_generator_set_generate_chunk(struct voxy_chunk_generator *chunk_generator, voxy_generate_chunk_t generate_chunk)
{
  chunk_generator->generate_chunk = generate_chunk;
}

struct voxy_chunk *voxy_chunk_generator_generate(struct voxy_chunk_generator *chunk_generator, ivec3_t position, const struct voxy_context *context)
{
  struct voxy_chunk *chunk = voxy_chunk_create();
  chunk->position = position;
  chunk->disk_dirty = true;
  chunk->network_dirty = true;
  chunk_generator->generate_chunk(position, chunk, chunk_generator->seed, context);
  return chunk;
}

