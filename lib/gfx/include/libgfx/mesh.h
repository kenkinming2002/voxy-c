#ifndef LIBCOMMON_GRAPHICS_MESH_H
#define LIBCOMMON_GRAPHICS_MESH_H

#include <libmath/vector.h>
#include <libmath/matrix.h>

#include <glad/glad.h>

// We make the simplifying assumption that our mesh are formed from vertices
// of 3d position, 3d normal and 2d texture coordinates.
struct mesh_vertex
{
  fvec3_t position;
  fvec3_t normal;
  fvec2_t uv;
};

struct mesh_vertex_instanced
{
  fmat4_t transform;
  float light;
};

struct mesh
{
  GLuint vao;

  GLuint vbo;
  GLuint vbo_instanced;

  size_t count;
};

int mesh_init(struct mesh *mesh);
void mesh_fini(struct mesh *mesh);

void mesh_update(struct mesh *mesh, const struct mesh_vertex *vertices, size_t vertex_count);
void mesh_update_instanced(const struct mesh *mesh, const struct mesh_vertex_instanced *vertices, size_t vertex_count);

int mesh_load(struct mesh *mesh, const char *filepath);

#endif // LIBCOMMON_GRAPHICS_MESH_H
