#ifndef CHUNK_COORDINATES_H
#define CHUNK_COORDINATES_H

#include <libmath/vector.h>

/// Coordinate conversions.
///
/// FIXME: This probably should be moved to some other module.
ivec3_t get_chunk_position_i(ivec3_t position);
ivec3_t get_chunk_position_f(fvec3_t position);

ivec3_t global_position_to_local_position_i(ivec3_t position);
fvec3_t global_position_to_local_position_f(fvec3_t position);

ivec3_t local_position_to_global_position_i(ivec3_t position, ivec3_t chunk_position);
fvec3_t local_position_to_global_position_f(fvec3_t position, ivec3_t chunk_position);

#endif // CHUNK_COORDINATES_H
