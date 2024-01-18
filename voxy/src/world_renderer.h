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

/************
 * Renderer *
 ************/
struct world_renderer
{
  struct resource_pack resource_pack;

  GLuint                       chunk_program;
  GLuint                       chunk_block_texture_array;
  struct chunk_mesh_hash_table chunk_meshes;
};

int world_renderer_init(struct world_renderer *world_renderer);
void world_renderer_deinit(struct world_renderer *world_renderer);

void world_renderer_update(struct world_renderer *world_renderer, struct world *world);
void world_renderer_render(struct world_renderer *world_renderer, struct camera *camera);

#endif // VOXY_WORLD_RENDERER_H
