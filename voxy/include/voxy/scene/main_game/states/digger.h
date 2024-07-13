#ifndef DIGGER_H
#define DIGGER_H

#include <voxy/math/vector.h>

/// Maintain state of blocks beging digged. This is more memory efficient than
/// simply storing digging state in each block since the number of block being
/// digged concurrently should be few (roughly limited to number of player).

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

#endif // DIGGER_H
