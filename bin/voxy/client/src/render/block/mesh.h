#ifndef RENDER_BLOCK_MESH_H
#define RENDER_BLOCK_MESH_H

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>
#include <glad/glad.h>

struct block_vertex
{
  ivec3_t center;

  // Bits 0..2: normal index
  // Bits 3.15: texture index
  uint32_t normal_index_and_texture_index;

  // Bits 0..3:   base light level
  // Bits 4..7:   occlusion count 0
  // Bits 8..11:  occlusion count 1
  // Bits 12..15: occlusion count 2
  // Bits 16..20: occlusion count 3
  uint32_t light_level_and_occlusion_counts;

  float damage;
};
DYNAMIC_ARRAY_DEFINE(block_vertices, struct block_vertex);

struct block_mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

void block_mesh_init(struct block_mesh *block_mesh);
void block_mesh_fini(struct block_mesh *block_mesh);

void block_mesh_update(struct block_mesh *block_mesh, struct block_vertices vertices);
void block_mesh_render(const struct block_mesh *block_mesh);

#endif // RENDER_BLOCK_MESH_H
