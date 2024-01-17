#ifndef VOXY_WORLD_RENDERER_H
#define VOXY_WORLD_RENDERER_H

#include <voxy/resource_pack.h>

#include "camera.h"
#include "glad/glad.h"
#include "world.h"

#define SC_HASH_TABLE_INTERFACE
#define SC_HASH_TABLE_PREFIX chunk_mesh
#define SC_HASH_TABLE_NODE_TYPE struct chunk_mesh
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_INTERFACE

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
void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency, struct ivec3 cposition, struct ivec3 dcposition);

/***************
 * Chunk Mesh *
 **************/
struct chunk_mesh
{
  struct chunk_mesh *next;
  size_t             hash;

  struct ivec3 position;

  GLuint vao;
  GLuint vbo;
  GLuint ibo;
  GLsizei count;
};

void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency);

/************
 * Renderer *
 ************/
struct world_renderer
{
  struct resource_pack resource_pack;

  GLuint chunk_program;
  GLuint chunk_block_texture_array;

  struct chunk_mesh_hash_table chunk_meshes;
};

int world_renderer_init(struct world_renderer *world_renderer);
void world_renderer_deinit(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct world *world);
void world_renderer_update_load(struct world_renderer *world_renderer, struct world *world);
void world_renderer_update_unload(struct world_renderer *world_renderer, struct world *world);
void world_renderer_update_reload(struct world_renderer *world_renderer, struct world *world);

void world_renderer_render(struct world_renderer *world_renderer, struct camera *camera);

#endif // VOXY_WORLD_RENDERER_H
