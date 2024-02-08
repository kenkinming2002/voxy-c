#ifndef TYPES_CHUNK_H
#define TYPES_CHUNK_H

#include <voxy/math/vector.h>

#include <stddef.h>
#include <stdbool.h>

struct chunk_data;
struct chunk_mesh;
struct chunk
{
  struct chunk *next;
  size_t        hash;
  ivec3_t       position;

  struct chunk *bottom;
  struct chunk *top;
  struct chunk *back;
  struct chunk *front;
  struct chunk *left;
  struct chunk *right;

  struct chunk_data *chunk_data;
  struct chunk_mesh *chunk_mesh;

  bool mesh_dirty;
  bool light_dirty;
};

#endif // TYPES_CHUNK_H

