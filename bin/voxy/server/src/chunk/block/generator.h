#ifndef CHUNK_BLOCK_GENERATOR_H
#define CHUNK_BLOCK_GENERATOR_H

#include "future.h"

#include <voxy/server/chunk/block/generator.h>

void voxy_block_generator_init(const char *world_directory);

struct block_group_future voxy_block_group_generate(ivec3_t position);

#endif // CHUNK_BLOCK_GENERATOR_H
