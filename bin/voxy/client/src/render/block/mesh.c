#include "mesh.h"

#include <libcore/log.h>

void block_mesh_init(struct block_mesh *block_mesh)
{
  glGenVertexArrays(1, &block_mesh->vao);
  glGenBuffers(1, &block_mesh->vbo);
  block_mesh->count = 0;
}

void block_mesh_fini(struct block_mesh *block_mesh)
{
  glDeleteVertexArrays(1, &block_mesh->vao);
  glDeleteBuffers(1, &block_mesh->vbo);
}

void block_mesh_update(struct block_mesh *block_mesh, struct block_vertices vertices)
{
  glBindVertexArray(block_mesh->vao);

  // Vertices
  {
    glBindBuffer(GL_ARRAY_BUFFER, block_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.item_count * sizeof *vertices.items, vertices.items, GL_DYNAMIC_DRAW);
  }

  // Indices
  {
    static GLuint ebo = 0;
    if(ebo == 0)
    {
      static uint8_t indices[] = { 0, 2, 1, 1, 2, 3 };
      glGenBuffers(1, &ebo);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
    }
    else
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  }

  // Vertex attributes
  {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(struct block_vertex), (void *)offsetof(struct block_vertex, center));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(struct block_vertex), (void *)offsetof(struct block_vertex, metadata1));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(struct block_vertex), (void *)offsetof(struct block_vertex, metadata2));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct block_vertex), (void *)offsetof(struct block_vertex, damage));

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
  }

  block_mesh->count = vertices.item_count;
}

void block_mesh_render(const struct block_mesh *block_mesh)
{
  if(block_mesh->count != 0)
  {
    glBindVertexArray(block_mesh->vao);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, block_mesh->count);
  }
}
