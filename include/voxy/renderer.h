#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include <voxy/world.h>
#include <voxy/camera.h>

#include <glad/glad.h>
#include <stddef.h>

/***************
 * Chunk Mesh *
 **************/
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

int chunk_mesh_init(struct chunk_mesh *chunk_mesh);
void chunk_mesh_deinit(struct chunk_mesh *chunk_mesh);
void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk *chunk);

/******************
 * Chunk Renderer *
 ******************/
struct chunk_renderer
{
  GLuint chunk_program;
};

int chunk_renderer_init(struct chunk_renderer *chunk_renderer);
void chunk_renderer_deinit(struct chunk_renderer *chunk_renderer);

void chunk_renderer_begin(struct chunk_renderer *chunk_renderer, struct camera *camera);
void chunk_renderer_render(struct chunk_renderer *chunk_renderer, struct chunk_mesh *chunk_mesh);

/**********
 * Skybox *
 **********/
struct skybox
{
  GLuint skybox_texture;
};

int skybox_load(struct skybox *skybox, const char *filepaths[6]);
void skybox_unload(struct skybox *skybox);

/*******************
 * Skybox Renderer *
 *******************/
struct skybox_renderer
{
  GLuint skybox_program;
  GLuint skybox_vao;
  GLuint skybox_vbo;
  GLuint skybox_ibo;
};

int skybox_renderer_init(struct skybox_renderer *skybox_renderer);
void skybox_renderer_deinit(struct skybox_renderer *skybox_renderer);

void skybox_renderer_render(struct skybox_renderer *skybox_renderer, struct camera *camera, struct skybox *skybox);

/*******************
 * Skybox Renderer *
 *******************/
struct renderer
{
  struct skybox_renderer skybox_renderer;
  struct skybox          skybox;

  struct chunk_renderer  chunk_renderer;
  struct chunk_mesh     *chunk_meshes;
  size_t                 chunk_mesh_count;
  size_t                 chunk_mesh_capacity;
};

int renderer_init(struct renderer *renderer);
void renderer_deinit(struct renderer *renderer);

struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, int z, int y, int x);
struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int z, int y, int x);

void renderer_update(struct renderer *renderer, struct world *world);
void renderer_render(struct renderer *renderer, struct camera *camera);

#endif // VOXY_RENDERER_H
