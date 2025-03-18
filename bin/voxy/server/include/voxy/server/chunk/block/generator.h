#ifndef VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H
#define VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H

#include <voxy/server/export.h>

#include <libmath/random.h>
#include <libmath/vector.h>

struct voxy_block_generator;
struct voxy_context;
struct voxy_block_group;

/// Type of callback function used to generate block.
typedef void(*voxy_generate_block_t)(ivec3_t block_position, struct voxy_block_group *block_group, seed_t seed, const struct voxy_context *context);

/// Set the function used to generate block.
VOXY_SERVER_EXPORT void voxy_block_generator_set_generate_block(struct voxy_block_generator *block_generator, voxy_generate_block_t generate_block);

#endif // VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H
