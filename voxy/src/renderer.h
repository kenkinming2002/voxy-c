#ifndef VOXY_RENDERER_H
#define VOXY_RENDERER_H

#include <voxy/resource_pack.h>

#include "camera.h"
#include "glad/glad.h"
#include "world.h"

#include <stddef.h>

/*****************
 * Resource Pack *
 *****************/
struct resource_pack
{
  void *handle;

  const struct block_info         *block_infos;
  const struct block_texture_info *block_texture_infos;

  size_t block_info_count;
  size_t block_texture_info_count;
};

int resource_pack_load(struct resource_pack *resource_pack, const char *filepath);
void resource_pack_unload(struct resource_pack *resource_pack);

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
void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency, int x, int y, int z, int dx, int dy, int dz);

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
void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency);

/************
 * Renderer *
 ************/
struct renderer
{
  struct resource_pack resource_pack;

  GLuint chunk_program;
  GLuint chunk_block_texture_array;

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
