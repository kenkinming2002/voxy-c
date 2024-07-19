#ifndef VOXY_GRAPHICS_MESH_H
#define VOXY_GRAPHICS_MESH_H

#include <glad/glad.h>

struct mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

int mesh_load(struct mesh *mesh, const char *filepath);
void mesh_unload(struct mesh *mesh);

#endif // VOXY_GRAPHICS_MESH_H
