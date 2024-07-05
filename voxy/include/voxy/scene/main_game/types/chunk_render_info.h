#ifndef VOXY_MAIN_GAME_TYPES_CHUNK_RENDER_INFO_H
#define VOXY_MAIN_GAME_TYPES_CHUNK_RENDER_INFO_H

#include <glad/glad.h>
#include <stdbool.h>

struct chunk_mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

struct chunk_render_info
{
  bool culled;
  struct chunk_mesh opaque_mesh;
  struct chunk_mesh transparent_mesh;
};

#endif // VOXY_MAIN_GAME_TYPES_CHUNK_RENDER_INFO_H
