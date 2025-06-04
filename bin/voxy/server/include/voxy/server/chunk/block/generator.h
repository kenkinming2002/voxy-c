#ifndef VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H
#define VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H

#include <voxy/config.h>
#include <voxy/server/export.h>
#include <voxy/server/registry/block.h>

#include <libmath/random.h>
#include <libmath/vector.h>

/// Type of callback function used to generate block.
typedef void(*voxy_generate_block_t)(seed_t seed, ivec3_t chunk_position, voxy_block_id_t blocks[VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH][VOXY_CHUNK_WIDTH]);

/// Set the function used to generate block.
VOXY_SERVER_EXPORT void voxy_set_generate_block(voxy_generate_block_t generate_block);

#endif // VOXY_SERVER_CHUNK_BLOCK_GENERATOR_H
