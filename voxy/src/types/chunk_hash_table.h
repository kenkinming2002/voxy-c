#ifndef TYPES_CHUNK_HASH_TABLE_H
#define TYPES_CHUNK_HASH_TABLE_H

#include <voxy/math/vector.h>

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk
#define SC_HASH_TABLE_NODE_TYPE struct chunk
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

#endif // TYPES_CHUNK_HASH_TABLE_H
