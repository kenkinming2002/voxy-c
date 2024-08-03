#ifndef VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_MESH_H
#define VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_MESH_H

#include <voxy/math/vector.h>
#include <voxy/dynamic_array.h>
#include <glad/glad.h>

struct vertex
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
DYNAMIC_ARRAY_DEFINE(vertices, struct vertex);

struct blocks_mesh
{
  GLuint vao;
  GLuint vbo;
  GLsizei count;
};

void blocks_mesh_init(struct blocks_mesh *blocks_mesh);
void blocks_mesh_fini(struct blocks_mesh *blocks_mesh);

void blocks_mesh_update(struct blocks_mesh *blocks_mesh, struct vertices vertices);
void blocks_mesh_render(const struct blocks_mesh *blocks_mesh);

#endif // VOXY_SCENE_MAIN_GAME_RENDER_BLOCKS_MESH_H
