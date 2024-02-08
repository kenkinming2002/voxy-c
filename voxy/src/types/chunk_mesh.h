#ifndef TYPES_CHUNK_MESH_H
#define TYPES_CHUNK_MESH_H

#include <voxy/config.h>
#include <glad/glad.h>

struct chunk_mesh
{
  GLuint vao_opaque;
  GLuint vbo_opaque;
  GLuint ibo_opaque;
  GLsizei count_opaque;

  GLuint vao_transparent;
  GLuint vbo_transparent;
  GLuint ibo_transparent;
  GLsizei count_transparent;
};

void chunk_mesh_dispose(struct chunk_mesh *chunk_mesh);

#endif // TYPES_CHUNK_MESH_H



