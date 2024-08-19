#ifndef VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATA_H
#define VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATA_H

#include <libcommon/math/vector.h>
#include <stdint.h>

/// Block data
///
/// Unlike block ids and light levels, we will not store them in a CHUNK_WIDTH x
/// CHUNK_WIDTH x CHUNK_WIDTH array as that is wasteful. This is because most of
/// the block in a chunk do not need associate data.
///
/// Instead, we use some form of sparse array format where we store the
/// position of block along with its data in a dynamic array.
struct block_data
{
  uint8_t x : 4;
  uint8_t y : 4;
  uint8_t z : 4;

  void *data;
};

ivec3_t block_data_position(const struct block_data *block_data);

#endif // VOXY_SCENE_MAIN_GAME_TYPES_BLOCK_DATA_H
