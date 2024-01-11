#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include <voxy/world.h>
#include <voxy/camera.h>

#include <glad/glad.h>
#include <stddef.h>

struct chunk_mesh
{
  int z;
  int y;
  int x;

  GLuint vao;
  GLuint vbo;
  GLuint ibo;
  GLsizei count;
};

struct renderer
{
  GLuint skybox_program;
  GLuint skybox_texture;
  GLuint skybox_vao;
  GLuint skybox_vbo;
  GLuint skybox_ibo;

  // FIXME: Use a proper hash table
  GLuint             chunk_program;
  struct chunk_mesh *chunk_meshes;
  size_t             chunk_mesh_count;
  size_t             chunk_mesh_capacity;
};

int renderer_init(struct renderer *renderer);

struct chunk_mesh *renderer_chunk_mesh_add   (struct renderer *renderer, struct chunk_mesh chunk_mesh);
struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int z, int y, int x);

void renderer_update(struct renderer *renderer, struct world *world);
void renderer_render(struct renderer *renderer, struct camera *camera);

#endif // VOXY_RENDERER_H
