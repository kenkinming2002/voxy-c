#ifndef CHUNK_BLOCK_GENERATOR_H
#define CHUNK_BLOCK_GENERATOR_H

#include "future.h"

#include <voxy/server/chunk/block/generator.h>

struct voxy_block_group;
struct voxy_block_generator;

struct block_generator_wrappers
{
  ivec3_t key;
  struct block_generator_job *value;
};

struct voxy_block_generator
{
  seed_t seed;
  voxy_generate_block_t generate_block;

  struct block_generator_wrappers *wrappers;
};

void voxy_block_generator_init(struct voxy_block_generator *block_generator, const char *world_directory);
void voxy_block_generator_fini(struct voxy_block_generator *block_generator);

struct block_group_future voxy_block_generator_generate(struct voxy_block_generator *block_generator, ivec3_t position, const struct voxy_context *context);

#endif // CHUNK_BLOCK_GENERATOR_H
