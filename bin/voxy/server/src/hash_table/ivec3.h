#ifndef HASH_TABLE_IVEC3_H
#define HASH_TABLE_IVEC3_H

#include <libmath/vector.h>

struct ivec3_node
{
  struct ivec3_node *next;
  size_t hash;
  ivec3_t key;
};

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX ivec3
#define SC_HASH_TABLE_NODE_TYPE struct ivec3_node
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_INTERFACE

#endif // HASH_TABLE_IVEC3_H
