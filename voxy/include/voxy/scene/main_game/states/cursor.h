#ifndef VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H
#define VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H

#include <voxy/scene/main_game/types/registry.h>

#include <voxy/math/vector.h>
#include <voxy/math/direction.h>

#include <stdbool.h>
#include <stdint.h>

struct chunk;

/// A simple cursor type which refer to a block in the world. It is possible to
/// move by one block in 6 axial direction by calling cursor_move(), which is
/// more efficient computing the new position and doing new lookup.
///
/// We make the decision to store x, y, z in bitfields. This is to save memory
/// usage as cursors are stored in fifo queues in the lighting system.
struct cursor
{
  struct chunk *chunk;
  uint8_t x : 4;
  uint8_t y : 4;
  uint8_t z : 4;
};

// Get the cursor to a block at position specified in global coordinate.
bool cursor_at(ivec3_t position, struct cursor *cursor);

// Get the chunk/local/global position of the block within the chunk.
ivec3_t cursor_get_chunk_position(struct cursor cursor);
ivec3_t cursor_get_local_position(struct cursor cursor);
ivec3_t cursor_get_global_position(struct cursor cursor);

/// Move the cursor by one block along the given direction.
bool cursor_move(struct cursor *cursor, direction_t direction);

/// Get/set block id.
block_id_t cursor_get_block_id(struct cursor cursor);
void cursor_set_block_id(struct cursor cursor, block_id_t id);

/// Get/set block light level.
unsigned cursor_get_block_light_level(struct cursor cursor);
void cursor_set_block_light_level(struct cursor cursor, unsigned light_level);

/// Get/set block light level atomically.
void cursor_get_block_light_level_atomic(struct cursor cursor, unsigned *light_level, unsigned char *tmp);
bool cursor_set_block_light_level_atomic(struct cursor cursor, unsigned *light_level, unsigned char *tmp);

#endif // VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H
