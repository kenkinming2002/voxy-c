#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include <voxy/world.h>
#include <voxy/camera.h>

#include <glad/glad.h>
#include <stddef.h>

/*******************
 * Chunk Adjacency *
 *******************/
struct chunk_adjacency
{
  struct chunk *chunk;

  struct chunk *bottom;
  struct chunk *top;

  struct chunk *back;
  struct chunk *front;

  struct chunk *left;
  struct chunk *right;
};

void chunk_adjacency_init(struct chunk_adjacency *chunk_adjacency, struct world *world, struct chunk *chunk);
struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, int x, int y, int z);

/**********************
 * Chunk Mesh Builder *
 **********************/
struct chunk_mesh_vertex
{
  struct vec3 position;
  struct vec2 texture_coords;
  uint32_t    texture_index;
};

struct chunk_mesh_builder
{
  struct chunk_mesh_vertex *vertices;
  size_t                    vertex_count;
  size_t                    vertex_capacity;

  uint32_t *indices;
  size_t    index_count;
  size_t    index_capacity;
};

void chunk_mesh_builder_init(struct chunk_mesh_builder *builder);
void chunk_mesh_builder_deinit(struct chunk_mesh_builder *builder);

void chunk_mesh_builder_reset(struct chunk_mesh_builder *builder);
void chunk_mesh_builder_push_vertex(struct chunk_mesh_builder *builder, struct chunk_mesh_vertex vertex);
void chunk_mesh_builder_push_index(struct chunk_mesh_builder *builder, uint32_t index);
void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_adjacency *chunk_adjacency, int x, int y, int z, int dx, int dy, int dz);

/***************
 * Chunk Mesh *
 **************/
struct chunk_mesh
{
  struct chunk_mesh *next;
  size_t             hash;

  int x, y, z;

  GLuint vao;
  GLuint vbo;
  GLuint ibo;
  GLsizei count;
};

void chunk_mesh_init(struct chunk_mesh *chunk_mesh);
void chunk_mesh_deinit(struct chunk_mesh *chunk_mesh);
void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_adjacency *chunk_adjacency);

/************
 * Renderer *
 ************/
struct renderer
{
  GLuint             chunk_program;
  GLuint             chunk_block_texture_array;

  struct chunk_mesh **chunk_meshes;
  size_t              chunk_mesh_capacity;
  size_t              chunk_mesh_load;
};

int renderer_init(struct renderer *renderer);
void renderer_deinit(struct renderer *renderer);

void renderer_chunk_mesh_rehash(struct renderer *renderer, size_t new_capacity);
struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, int x, int y, int z);
struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int x, int y, int z);
struct chunk_mesh *renderer_chunk_mesh_lookup_or_add(struct renderer *renderer, int x, int y, int z);

void renderer_update(struct renderer *renderer, struct world *world);
void renderer_render(struct renderer *renderer, struct camera *camera);

#endif // VOXY_RENDERER_H
