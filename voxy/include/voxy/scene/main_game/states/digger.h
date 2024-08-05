#ifndef VOXY_SCENE_MAIN_GAME_STATES_DIGGER_H
#define VOXY_SCENE_MAIN_GAME_STATES_DIGGER_H

#include <voxy/math/vector.h>

/// Maintain state of blocks beging digged. This is more memory efficient than
/// simply storing digging state in each block since the number of block being
/// digged concurrently should be few (roughly limited to number of player).

struct entity;

/// The digger struct.
///
/// We are storing a pointer to the block being digged directly. This is fine
/// because we never ever actually dereference the pointer.
struct digger
{
  ivec3_t position;
  float damage;
};

/// Currently, there can only be one digger as there is only one player(sad) but
/// in the future this should be some form of a dynamic array.
extern struct digger g_digger;

/// Reset the digger.
void digger_reset(struct digger *digger);

/// Set digger position. If this differ from previously set position, the digger
/// is reset, and all previous progress is lost.
void digger_set_position(struct digger *digger, ivec3_t position);

/// Actually Dig. This will destroy the block at previously set position if
/// accumulated damage is greater than 1.0.
bool digger_dig(struct digger *digger, float damage);

#endif // VOXY_SCENE_MAIN_GAME_STATES_DIGGER_H
