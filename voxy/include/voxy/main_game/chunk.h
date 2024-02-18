#ifndef TYPES_CHUNK_H
#define TYPES_CHUNK_H

#include <voxy/main_game/block.h>
#include <voxy/main_game/config.h>
#include <voxy/math/vector.h>

#include <glad/glad.h>

#include <stddef.h>
#include <stdbool.h>

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

  bool          mesh_invalidated;
  struct chunk *mesh_next;

  bool          light_invalidated;
  struct chunk *light_next;

  struct block blocks[CHUNK_WIDTH][CHUNK_WIDTH][CHUNK_WIDTH];

  GLuint vao_opaque;
  GLuint vbo_opaque;
  GLsizei count_opaque;

  GLuint vao_transparent;
  GLuint vbo_transparent;
  GLsizei count_transparent;
};

#endif // TYPES_CHUNK_H

