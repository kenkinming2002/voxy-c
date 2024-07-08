#ifndef VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H
#define VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H

#include <voxy/math/vector.h>
#include <voxy/math/direction.h>

#include <stdbool.h>
#include <stdint.h>

struct chunk;

/// A simple cursor type which refer to a block in the world. It is possible to
/// move by one block in 6 axial direction by calling cursor_move(), which is
/// more efficient computing the new position and doing new lookup.
struct cursor
{
  struct chunk *chunk;
  uint8_t x : 4;
  uint8_t y : 4;
  uint8_t z : 4;
};

// Get the cursor to a block at position.
bool cursor_at(ivec3_t position, struct cursor *cursor);

// Get the block refered to by the cursor
struct block *cursor_get(struct cursor cursor);

/// Move the cursor by one block along the given direction.
bool cursor_move(struct cursor *cursor, enum direction direction);

#endif // VOXY_SCENE_MAIN_GAME_STATES_CURSOR_H
