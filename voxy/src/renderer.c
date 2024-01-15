#include "renderer.h"

#include "lin.h"
#include "gl.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

/*****************
 * Resource Pack *
 *****************/
int resource_pack_load(struct resource_pack *resource_pack, const char *filepath)
{
  size_t *value;

  if(!(resource_pack->handle = dlopen(filepath, RTLD_LAZY)))
  {
    fprintf(stderr, "ERROR: Failed to load resource pack from %s: %s\n", filepath, strerror(errno));
    goto error;
  }

  if(!(resource_pack->block_infos = dlsym(resource_pack->handle, "block_infos")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_infos from resource pack %s\n", filepath);
    goto error;
  }

  if(!(resource_pack->block_texture_infos = dlsym(resource_pack->handle, "block_texture_infos")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_texture_infos from resource pack %s\n", filepath);
    goto error;
  }

  if(!(value = dlsym(resource_pack->handle, "block_info_count")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_info_count from resource pack %s\n", filepath);
    goto error;
  }
  resource_pack->block_info_count = *value;

  if(!(value = dlsym(resource_pack->handle, "block_texture_info_count")))
  {
    fprintf(stderr, "ERROR: Missing symbol block_texture_info_count from resource pack %s\n", filepath);
    goto error;
  }
  resource_pack->block_texture_info_count = *value;

  return 0;

error:
  if(resource_pack->handle)
    dlclose(resource_pack->handle);

  return -1;
}

void resource_pack_unload(struct resource_pack *resource_pack)
{
  dlclose(resource_pack->handle);
}

/**********************
 * Chunk Mesh Builder *
 **********************/
void chunk_mesh_builder_init(struct chunk_mesh_builder *chunk_mesh_builder)
{
  chunk_mesh_builder->vertices        = NULL;
  chunk_mesh_builder->vertex_count    = 0;
  chunk_mesh_builder->vertex_capacity = 0;

  chunk_mesh_builder->indices        = NULL;
  chunk_mesh_builder->index_count    = 0;
  chunk_mesh_builder->index_capacity = 0;
}

void chunk_mesh_builder_deinit(struct chunk_mesh_builder *chunk_mesh_builder)
{
  free(chunk_mesh_builder->vertices);
  free(chunk_mesh_builder->indices);
}

void chunk_mesh_builder_reset(struct chunk_mesh_builder *chunk_mesh_builder)
{
  chunk_mesh_builder->vertex_count = 0;
  chunk_mesh_builder->index_count  = 0;
}

void chunk_mesh_builder_push_vertex(struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_mesh_vertex vertex)
{
  if(chunk_mesh_builder->vertex_capacity == chunk_mesh_builder->vertex_count)
  {
    chunk_mesh_builder->vertex_capacity = chunk_mesh_builder->vertex_capacity != 0 ? chunk_mesh_builder->vertex_capacity * 2 : 1;
    chunk_mesh_builder->vertices        = realloc(chunk_mesh_builder->vertices, chunk_mesh_builder->vertex_capacity * sizeof *chunk_mesh_builder->vertices);
  }
  chunk_mesh_builder->vertices[chunk_mesh_builder->vertex_count++] = vertex;
}

void chunk_mesh_builder_push_index(struct chunk_mesh_builder *chunk_mesh_builder, uint32_t index)
{
  if(chunk_mesh_builder->index_capacity == chunk_mesh_builder->index_count)
  {
    chunk_mesh_builder->index_capacity = chunk_mesh_builder->index_capacity != 0 ? chunk_mesh_builder->index_capacity * 2 : 1;
    chunk_mesh_builder->indices        = realloc(chunk_mesh_builder->indices, chunk_mesh_builder->index_capacity * sizeof *chunk_mesh_builder->indices);
  }
  chunk_mesh_builder->indices[chunk_mesh_builder->index_count++] = index;
}

void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency, int x, int y, int z, int dx, int dy, int dz)
{
  struct tile *ntile = chunk_adjacency_tile_lookup(chunk_adjacency, x+dx, y+dy, z+dz);
  if(ntile && ntile->id != TILE_ID_EMPTY)
    return; // Occlusion

  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 0);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 1);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 2);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 2);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 1);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 3);

  // Fancy way to compute vertex positions without just dumping a big table here
  // Pray that the compiler will just inline everything (With -ffast-math probably)

  struct vec3 normal = vec3(dx, dy, dz);
  struct vec3 axis2  = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 axis1  = vec3_cross(normal, axis2);

  struct chunk_mesh_vertex vertices[4];

  // 1: Position
  struct vec3 center = vec3_zero();;
  center = vec3_add  (center, vec3(chunk_adjacency->chunk->x, chunk_adjacency->chunk->y, chunk_adjacency->chunk->z));
  center = vec3_mul_s(center, CHUNK_WIDTH);
  center = vec3_add  (center, vec3(x, y, z));
  center = vec3_add  (center, vec3(0.5f, 0.5f, 0.5f));
  center = vec3_add  (center, vec3_mul_s(normal, 0.5f));

  vertices[0].position = vec3_add(center, vec3_add(vec3_mul_s(axis1, -0.5f), vec3_mul_s(axis2, -0.5f)));
  vertices[1].position = vec3_add(center, vec3_add(vec3_mul_s(axis1, -0.5f), vec3_mul_s(axis2,  0.5f)));
  vertices[2].position = vec3_add(center, vec3_add(vec3_mul_s(axis1,  0.5f), vec3_mul_s(axis2, -0.5f)));
  vertices[3].position = vec3_add(center, vec3_add(vec3_mul_s(axis1,  0.5f), vec3_mul_s(axis2,  0.5f)));

  // 2: Texture Coords
  vertices[0].texture_coords = vec2(0.0f, 0.0f);
  vertices[1].texture_coords = vec2(0.0f, 1.0f);
  vertices[2].texture_coords = vec2(1.0f, 0.0f);
  vertices[3].texture_coords = vec2(1.0f, 1.0f);

  // 3: Texture Index
  uint32_t texture_index;
  if     (dx == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_left;
  else if(dx ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_right;
  else if(dy == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_back;
  else if(dy ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_front;
  else if(dz == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_bottom;
  else if(dz ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[z][y][x].id].texture_top;
  else
    assert(0 && "Unreachable");

  vertices[0].texture_index = texture_index;
  vertices[1].texture_index = texture_index;
  vertices[2].texture_index = texture_index;
  vertices[3].texture_index = texture_index;

  // 4: Push
  chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertices[0]);
  chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertices[1]);
  chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertices[2]);
  chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertices[3]);
}


/***************
 * Chunk Mesh *
 **************/
void chunk_mesh_init(struct chunk_mesh *chunk_mesh)
{
  glGenVertexArrays(1, &chunk_mesh->vao);
  glGenBuffers(1, &chunk_mesh->vbo);
  glGenBuffers(1, &chunk_mesh->ibo);
}

void chunk_mesh_deinit(struct chunk_mesh *chunk_mesh)
{
  glDeleteVertexArrays(1, &chunk_mesh->vao);
  glDeleteBuffers(1, &chunk_mesh->vbo);
  glDeleteBuffers(1, &chunk_mesh->ibo);
}

void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency)
{
  chunk_mesh_builder_reset(chunk_mesh_builder);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk_adjacency->chunk->tiles[z][y][x].id != TILE_ID_EMPTY)
        {
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z, -1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z,  1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z,  0, -1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z,  0,  1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z,  0,  0, -1);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, x, y, z,  0,  0,  1);
        }

  glBindVertexArray(chunk_mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, chunk_mesh_builder->vertex_count * sizeof *chunk_mesh_builder->vertices, chunk_mesh_builder->vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_builder->index_count * sizeof *chunk_mesh_builder->indices, chunk_mesh_builder->indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);
  glEnableVertexAttribArray(2);

  glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
  glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
  glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));

  chunk_mesh->count = chunk_mesh_builder->index_count;
}

/************
 * Renderer *
 ************/
int renderer_init(struct renderer *renderer)
{
  if(resource_pack_load(&renderer->resource_pack, "resource_pack/resource_pack.so") != 0)
    return -1;

  renderer->chunk_program             = 0;
  renderer->chunk_block_texture_array = 0;
  renderer->chunk_meshes              = NULL;
  renderer->chunk_mesh_capacity       = 0;
  renderer->chunk_mesh_load           = 0;

  if((renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load chunk shader\n");
    goto error;
  }

  size_t       filepath_count = renderer->resource_pack.block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);

  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = renderer->resource_pack.block_texture_infos[i].filepath;
  renderer->chunk_block_texture_array = gl_array_texture_load(filepath_count, filepaths);

  free(filepaths);

  if(renderer->chunk_block_texture_array == 0)
  {
    fprintf(stderr, "ERROR: Failed to load block textures\n");
    goto error;
  }

  return 0;

error:
  if(renderer->chunk_program != 0)
    glDeleteProgram(renderer->chunk_program);

  if(renderer->chunk_block_texture_array != 0)
    glDeleteTextures(1, &renderer->chunk_block_texture_array);

  resource_pack_unload(&renderer->resource_pack);
  return -1;
}

void renderer_deinit(struct renderer *renderer)
{
  glDeleteProgram(renderer->chunk_program);
  glDeleteTextures(1, &renderer->chunk_block_texture_array);

  for(size_t i=0; i<renderer->chunk_mesh_capacity; ++i)
  {
    struct chunk_mesh *chunk_mesh = renderer->chunk_meshes[i];
    while(chunk_mesh)
    {
      struct chunk_mesh *tmp = chunk_mesh;
      chunk_mesh = chunk_mesh->next;
      chunk_mesh_deinit(tmp);
      free(tmp);
    }
  }
  free(renderer->chunk_meshes);
}

static inline size_t hash(int x, int y, int z)
{
  // I honestly do not know what I am doing here
  return x * 23 + y * 31 + z * 47;
}

void renderer_chunk_mesh_rehash(struct renderer *renderer, size_t new_capacity)
{
  struct chunk_mesh *orphans = NULL;
  for(size_t i=0; i<renderer->chunk_mesh_capacity; ++i)
  {
    struct chunk_mesh **head = &renderer->chunk_meshes[i];
    while(*head)
      if((*head)->hash % new_capacity != i)
      {
        struct chunk_mesh *orphan = *head;
        *head = (*head)->next;

        orphan->next = orphans;
        orphans      = orphan;
      }
      else
        head = &(*head)->next;
  }

  renderer->chunk_meshes = realloc(renderer->chunk_meshes, new_capacity * sizeof *renderer->chunk_meshes);
  for(size_t i=renderer->chunk_mesh_capacity; i<new_capacity; ++i)
    renderer->chunk_meshes[i] = NULL;
  renderer->chunk_mesh_capacity = new_capacity;

  while(orphans)
  {
    struct chunk_mesh *orphan = orphans;
    orphans = orphans->next;

    orphan->next = renderer->chunk_meshes[orphan->hash % renderer->chunk_mesh_capacity];
    renderer->chunk_meshes[orphan->hash % renderer->chunk_mesh_capacity] = orphan;
  }
}

struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, int x, int y, int z)
{
  if(renderer->chunk_mesh_capacity == 0)
    renderer_chunk_mesh_rehash(renderer, 32);
  else if(renderer->chunk_mesh_load * 4 >= renderer->chunk_mesh_capacity * 3)
    renderer_chunk_mesh_rehash(renderer, renderer->chunk_mesh_capacity * 2);

  struct chunk_mesh *chunk_mesh = malloc(sizeof *chunk_mesh);
  chunk_mesh->x = x;
  chunk_mesh->y = y;
  chunk_mesh->z = z;
  chunk_mesh_init(chunk_mesh);

  chunk_mesh->hash = hash(x, y, z);
  chunk_mesh->next = renderer->chunk_meshes[chunk_mesh->hash % renderer->chunk_mesh_capacity];

  renderer->chunk_meshes[chunk_mesh->hash % renderer->chunk_mesh_capacity] = chunk_mesh;
  renderer->chunk_mesh_load += 1;

  return chunk_mesh;
}

struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int x, int y, int z)
{
  if(renderer->chunk_mesh_capacity == 0)
    return NULL;

  size_t h = hash(x, y, z);
  for(struct chunk_mesh *chunk_mesh = renderer->chunk_meshes[h % renderer->chunk_mesh_capacity]; chunk_mesh; chunk_mesh = chunk_mesh->next)
    if(chunk_mesh->hash == h && chunk_mesh->x == x && chunk_mesh->y == y && chunk_mesh->z == z)
      return chunk_mesh;
  return NULL;
}

struct chunk_mesh *renderer_chunk_mesh_lookup_or_add(struct renderer *renderer, int x, int y, int z)
{
  struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup(renderer, x, y, z);
  if(chunk_mesh)
    return chunk_mesh;
  else
    return renderer_chunk_mesh_add(renderer, x, y, z);
}

void renderer_update(struct renderer *renderer, struct world *world)
{
  struct chunk_mesh_builder chunk_mesh_builder;
  chunk_mesh_builder_init(&chunk_mesh_builder);
  while(world->chunk_remesh_list)
  {
    struct chunk *chunk = world->chunk_remesh_list;
    world->chunk_remesh_list = world->chunk_remesh_list->remesh_next;
    chunk->remesh = false;

    struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup_or_add(renderer, chunk->x, chunk->y, chunk->z);
    struct chunk_adjacency chunk_adjacency;
    chunk_adjacency_init(&chunk_adjacency, world, chunk);
    chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, &renderer->resource_pack, &chunk_adjacency);
  }
  chunk_mesh_builder_deinit(&chunk_mesh_builder);
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  struct mat4 VP = mat4_identity();
  VP = mat4_mul(camera_view_matrix(camera),       VP);
  VP = mat4_mul(camera_projection_matrix(camera), VP);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->chunk_block_texture_array);

  for(size_t i=0; i<renderer->chunk_mesh_capacity; ++i)
    for(struct chunk_mesh *chunk_mesh = renderer->chunk_meshes[i]; chunk_mesh; chunk_mesh = chunk_mesh->next)
    {
      glBindVertexArray(chunk_mesh->vao);
      glDrawElements(GL_TRIANGLES, chunk_mesh->count, GL_UNSIGNED_INT, 0);
    }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

