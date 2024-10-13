#ifndef VOXY_SERVER_CHUNK_GENERATOR_H
#define VOXY_SERVER_CHUNK_GENERATOR_H

#include <voxy/server/export.h>
#include <libcommon/math/vector.h>

struct voxy_chunk_generator;
struct voxy_context;
struct voxy_chunk;

/// Type of callback function used to generate chunk.
typedef void(*voxy_generate_chunk_t)(ivec3_t chunk_position, struct voxy_chunk *chunk, seed_t seed, const struct voxy_context *context);

/// Set the function used to generate chunk.
VOXY_SERVER_EXPORT void voxy_chunk_generator_set_generate_chunk(struct voxy_chunk_generator *chunk_generator, voxy_generate_chunk_t generate_chunk);

#endif // VOXY_SERVER_CHUNK_GENERATOR_H
