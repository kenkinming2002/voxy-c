#ifndef RENDER_BLOCKS_MESH_H
#define RENDER_BLOCKS_MESH_H

#include <libcommon/math/vector.h>
#include <libcommon/utils/dynamic_array.h>
#include <glad/glad.h>

struct blocks_vertex
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
DYNAMIC_ARRAY_DEFINE(blocks_vertices, struct blocks_vertex);

struct blocks_mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

void blocks_mesh_init(struct blocks_mesh *blocks_mesh);
void blocks_mesh_fini(struct blocks_mesh *blocks_mesh);

void blocks_mesh_update(struct blocks_mesh *blocks_mesh, struct blocks_vertices vertices);
void blocks_mesh_render(const struct blocks_mesh *blocks_mesh);

#endif // RENDER_BLOCKS_MESH_H
