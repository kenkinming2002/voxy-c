#include "blocks_mesh.h"

void blocks_mesh_init(struct blocks_mesh *blocks_mesh)
{
  glGenVertexArrays(1, &blocks_mesh->vao);
  glGenBuffers(1, &blocks_mesh->vbo);
  blocks_mesh->count = 0;
}

void blocks_mesh_fini(struct blocks_mesh *blocks_mesh)
{
  glDeleteVertexArrays(1, &blocks_mesh->vao);
  glDeleteBuffers(1, &blocks_mesh->vbo);
}

void blocks_mesh_update(struct blocks_mesh *blocks_mesh, struct vertices vertices)
{
  glBindVertexArray(blocks_mesh->vao);

  // Vertices
  {
    glBindBuffer(GL_ARRAY_BUFFER, blocks_mesh->vbo);
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

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(struct mesh_vertices), (void *)offsetof(struct mesh_vertices, center));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(struct mesh_vertices), (void *)offsetof(struct mesh_vertices, normal_index_and_texture_index));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(struct mesh_vertices), (void *)offsetof(struct mesh_vertices, light_level_and_occlusion_counts));
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(struct mesh_vertices), (void *)offsetof(struct mesh_vertices, damage));

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);
    glVertexAttribDivisor(3, 1);
  }

  blocks_mesh->count = vertices.item_count;
}

void blocks_mesh_render(const struct blocks_mesh *blocks_mesh)
{
  if(blocks_mesh->count != 0)
  {
    glBindVertexArray(blocks_mesh->vao);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, blocks_mesh->count);
  }
}
