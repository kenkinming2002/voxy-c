#ifndef RENDER_BLOCK_MESH_H
#define RENDER_BLOCK_MESH_H

#include <libmath/vector.h>
#include <glad/glad.h>

struct block_vertex
{
  // Center: 3 * 4 bits
  uint16_t center;

  // Light levels: 4 * 6 bits = 24 bits
  uint32_t metadata1;

  // Occlusion counts: 4 * 4 bits = 16 bits
  // Normal index:                  3  bits
  // Texture index:                 13 bits
  uint32_t metadata2;

  float damage;
};

struct block_mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

void block_mesh_init(struct block_mesh *block_mesh);
void block_mesh_fini(struct block_mesh *block_mesh);

void block_mesh_update(struct block_mesh *block_mesh, struct block_vertex *vertices);
void block_mesh_render(const struct block_mesh *block_mesh);

#endif // RENDER_BLOCK_MESH_H
