#include "world_renderer.h"

#include "lin.h"
#include "gl.h"
#include "hash.h"

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX chunk_mesh
#define SC_HASH_TABLE_NODE_TYPE struct chunk_mesh
#define SC_HASH_TABLE_KEY_TYPE struct ivec3
#include "hash_table.h"
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_IMPLEMENTATION

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>

struct ivec3 chunk_mesh_key(struct chunk_mesh *chunk_mesh)
{
  return chunk_mesh->position;
}

size_t chunk_mesh_hash(struct ivec3 position)
{
  return hash3(position.x, position.y, position.z);
}

int chunk_mesh_compare(struct ivec3 position1, struct ivec3 position2)
{
  if(position1.x != position2.x) return position1.x - position2.x;
  if(position1.y != position2.y) return position1.y - position2.y;
  if(position1.z != position2.z) return position1.z - position2.z;
  return 0;
}

void chunk_mesh_dispose(struct chunk_mesh *chunk_mesh)
{
  glDeleteVertexArrays(1, &chunk_mesh->vao);
  glDeleteBuffers(1, &chunk_mesh->vbo);
  glDeleteBuffers(1, &chunk_mesh->ibo);
  free(chunk_mesh);
}

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

void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency, struct ivec3 cposition, struct ivec3 dcposition)
{
  struct tile *ntile = chunk_adjacency_tile_lookup(chunk_adjacency, ivec3_add(cposition, dcposition));
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

  struct vec3 normal = vec3(dcposition.x, dcposition.y, dcposition.z);
  struct vec3 axis2  = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 axis1  = vec3_cross(normal, axis2);

  struct chunk_mesh_vertex vertices[4];

  // 1: Position
  struct vec3 center = vec3_zero();;
  center = vec3_add  (center, vec3(chunk_adjacency->chunk->position.x, chunk_adjacency->chunk->position.y, chunk_adjacency->chunk->position.z));
  center = vec3_mul_s(center, CHUNK_WIDTH);
  center = vec3_add  (center, vec3(cposition.x, cposition.y, cposition.z));
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
  if     (dcposition.x == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_left;
  else if(dcposition.x ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_right;
  else if(dcposition.y == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_back;
  else if(dcposition.y ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_front;
  else if(dcposition.z == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_bottom;
  else if(dcposition.z ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cposition.z][cposition.y][cposition.x].id].texture_top;
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
void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency)
{
  chunk_adjacency->chunk->mesh_dirty = false;

  chunk_mesh_builder_reset(chunk_mesh_builder);
  for(int cz = 0; cz<CHUNK_WIDTH; ++cz)
    for(int cy = 0; cy<CHUNK_WIDTH; ++cy)
      for(int cx = 0; cx<CHUNK_WIDTH; ++cx)
        if(chunk_adjacency->chunk->tiles[cz][cy][cx].id != TILE_ID_EMPTY)
        {
          struct ivec3 cposition = { .x = cx, .y = cy, .z = cz };
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3(-1,  0,  0));
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3( 1,  0,  0));
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3( 0, -1,  0));
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3( 0,  1,  0));
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3( 0,  0, -1));
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cposition, ivec3( 0,  0,  1));
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
int world_renderer_init(struct world_renderer *world_renderer)
{
  if(resource_pack_load(&world_renderer->resource_pack, "resource_pack/resource_pack.so") != 0)
    return -1;

  world_renderer->chunk_program             = 0;
  world_renderer->chunk_block_texture_array = 0;

  if((world_renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load chunk shader\n");
    goto error;
  }

  size_t       filepath_count = world_renderer->resource_pack.block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);

  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = world_renderer->resource_pack.block_texture_infos[i].filepath;
  world_renderer->chunk_block_texture_array = gl_array_texture_load(filepath_count, filepaths);

  free(filepaths);

  if(world_renderer->chunk_block_texture_array == 0)
  {
    fprintf(stderr, "ERROR: Failed to load block textures\n");
    goto error;
  }

  chunk_mesh_hash_table_init(&world_renderer->chunk_meshes);
  return 0;

error:
  if(world_renderer->chunk_program != 0)
    glDeleteProgram(world_renderer->chunk_program);

  if(world_renderer->chunk_block_texture_array != 0)
    glDeleteTextures(1, &world_renderer->chunk_block_texture_array);

  resource_pack_unload(&world_renderer->resource_pack);
  return -1;
}

void world_renderer_deinit(struct world_renderer *world_renderer)
{
  glDeleteProgram(world_renderer->chunk_program);
  glDeleteTextures(1, &world_renderer->chunk_block_texture_array);
  chunk_mesh_hash_table_dispose(&world_renderer->chunk_meshes);
}

void world_renderer_update(struct world_renderer *world_renderer, struct world *world)
{
  world_renderer_update_unload(world_renderer, world);
  world_renderer_update_load(world_renderer, world);
  world_renderer_update_reload(world_renderer, world);
}

#define RENDERER_LOAD_DISTANCE   16
#define RENDERER_UNLOAD_DISTANCE 64

void world_renderer_update_load(struct world_renderer *world_renderer, struct world *world)
{
  struct chunk_mesh_builder chunk_mesh_builder;
  chunk_mesh_builder_init(&chunk_mesh_builder);

  struct chunk           *chunk;
  struct chunk_mesh      *chunk_mesh;
  struct chunk_adjacency  chunk_adjacency;

  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);
  for(int dz = -RENDERER_LOAD_DISTANCE; dz<=RENDERER_LOAD_DISTANCE; ++dz)
    for(int dy = -RENDERER_LOAD_DISTANCE; dy<=RENDERER_LOAD_DISTANCE; ++dy)
      for(int dx = -RENDERER_LOAD_DISTANCE; dx<=RENDERER_LOAD_DISTANCE; ++dx)
      {
        struct ivec3 chunk_position = ivec3_add(ivec3(x, y, z), ivec3(dx, dy, dz));
        if(!(chunk = chunk_hash_table_lookup(&world->chunks, chunk_position)))
          continue;

        if(chunk_mesh_hash_table_lookup(&world_renderer->chunk_meshes, chunk_position))
          continue;

        chunk_mesh = malloc(sizeof *chunk_mesh);
        chunk_mesh->position = chunk_position;

        glGenVertexArrays(1, &chunk_mesh->vao);
        glGenBuffers(1, &chunk_mesh->vbo);
        glGenBuffers(1, &chunk_mesh->ibo);

        chunk_mesh_hash_table_insert_unchecked(&world_renderer->chunk_meshes, chunk_mesh);

        chunk_adjacency_init(&chunk_adjacency, world, chunk);
        chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, &world_renderer->resource_pack, &chunk_adjacency);
      }

  chunk_mesh_builder_deinit(&chunk_mesh_builder);
}

void world_renderer_update_unload(struct world_renderer *world_renderer, struct world *world)
{
  int x = floorf(world->player_transform.translation.x / CHUNK_WIDTH);
  int y = floorf(world->player_transform.translation.y / CHUNK_WIDTH);
  int z = floorf(world->player_transform.translation.z / CHUNK_WIDTH);

  for(size_t i=0; i<world_renderer->chunk_meshes.bucket_count; ++i)
    for(struct chunk_mesh **chunk_mesh = &world_renderer->chunk_meshes.buckets[i].head; *chunk_mesh;)
      if((*chunk_mesh)->position.x < x - RENDERER_UNLOAD_DISTANCE || (*chunk_mesh)->position.x > x + RENDERER_UNLOAD_DISTANCE ||
         (*chunk_mesh)->position.y < y - RENDERER_UNLOAD_DISTANCE || (*chunk_mesh)->position.y > y + RENDERER_UNLOAD_DISTANCE ||
         (*chunk_mesh)->position.z < z - RENDERER_UNLOAD_DISTANCE || (*chunk_mesh)->position.z > z + RENDERER_UNLOAD_DISTANCE)
      {
        struct chunk_mesh *tmp = *chunk_mesh;
        *chunk_mesh = (*chunk_mesh)->next;
        chunk_mesh_dispose(tmp);

        world_renderer->chunk_meshes.load -= 1;
      }
      else
        chunk_mesh = &(*chunk_mesh)->next;
}

void world_renderer_update_reload(struct world_renderer *world_renderer, struct world *world)
{
  struct chunk_mesh_builder chunk_mesh_builder;
  chunk_mesh_builder_init(&chunk_mesh_builder);
  for(size_t i=0; i<world_renderer->chunk_meshes.bucket_count; ++i)
    for(struct chunk_mesh *chunk_mesh = world_renderer->chunk_meshes.buckets[i].head; chunk_mesh; chunk_mesh = chunk_mesh->next)
    {
      struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_mesh->position);
      if(chunk && chunk->mesh_dirty)
      {
        struct chunk_adjacency chunk_adjacency;
        chunk_adjacency_init(&chunk_adjacency, world, chunk);
        chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, &world_renderer->resource_pack, &chunk_adjacency);
      }
    }

  chunk_mesh_builder_deinit(&chunk_mesh_builder);
}

void world_renderer_render(struct world_renderer *world_renderer, struct camera *camera)
{
  struct mat4 VP = mat4_identity();
  VP = mat4_mul(camera_view_matrix(camera),       VP);
  VP = mat4_mul(camera_projection_matrix(camera), VP);

  struct mat4 V = mat4_identity();
  V = mat4_mul(camera_view_matrix(camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(world_renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_CUBE_MAP, world_renderer->chunk_block_texture_array);

  for(size_t i=0; i<world_renderer->chunk_meshes.bucket_count; ++i)
    for(struct chunk_mesh *chunk_mesh = world_renderer->chunk_meshes.buckets[i].head; chunk_mesh; chunk_mesh = chunk_mesh->next)
    {
      glBindVertexArray(chunk_mesh->vao);
      glDrawElements(GL_TRIANGLES, chunk_mesh->count, GL_UNSIGNED_INT, 0);
    }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

