#include "world_renderer.h"

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

void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct resource_pack *resource_pack, struct chunk_adjacency *chunk_adjacency, int cx, int cy, int cz, int dcx, int dcy, int dcz)
{
  struct tile *ntile = chunk_adjacency_tile_lookup(chunk_adjacency, cx+dcx, cy+dcy, cz+dcz);
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

  struct vec3 normal = vec3(dcx, dcy, dcz);
  struct vec3 axis2  = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 axis1  = vec3_cross(normal, axis2);

  struct chunk_mesh_vertex vertices[4];

  // 1: Position
  struct vec3 center = vec3_zero();;
  center = vec3_add  (center, vec3(chunk_adjacency->chunk->x, chunk_adjacency->chunk->y, chunk_adjacency->chunk->z));
  center = vec3_mul_s(center, CHUNK_WIDTH);
  center = vec3_add  (center, vec3(cx, cy, cz));
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
  if     (dcx == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_left;
  else if(dcx ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_right;
  else if(dcy == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_back;
  else if(dcy ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_front;
  else if(dcz == -1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_bottom;
  else if(dcz ==  1) texture_index = resource_pack->block_infos[chunk_adjacency->chunk->tiles[cz][cy][cx].id].texture_top;
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
  chunk_adjacency->chunk->mesh_dirty = false;

  chunk_mesh_builder_reset(chunk_mesh_builder);
  for(int cz = 0; cz<CHUNK_WIDTH; ++cz)
    for(int cy = 0; cy<CHUNK_WIDTH; ++cy)
      for(int cx = 0; cx<CHUNK_WIDTH; ++cx)
        if(chunk_adjacency->chunk->tiles[cz][cy][cx].id != TILE_ID_EMPTY)
        {
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz, -1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz,  1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz,  0, -1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz,  0,  1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz,  0,  0, -1);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, resource_pack, chunk_adjacency, cx, cy, cz,  0,  0,  1);
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
  world_renderer->chunk_meshes              = NULL;
  world_renderer->chunk_mesh_capacity       = 0;
  world_renderer->chunk_mesh_load           = 0;

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

  for(size_t i=0; i<world_renderer->chunk_mesh_capacity; ++i)
  {
    struct chunk_mesh *chunk_mesh = world_renderer->chunk_meshes[i];
    while(chunk_mesh)
    {
      struct chunk_mesh *tmp = chunk_mesh;
      chunk_mesh = chunk_mesh->next;
      chunk_mesh_deinit(tmp);
      free(tmp);
    }
  }
  free(world_renderer->chunk_meshes);
}

static inline size_t hash(int x, int y, int z)
{
  // I honestly do not know what I am doing here
  return x * 23 + y * 31 + z * 47;
}

void world_renderer_chunk_mesh_rehash(struct world_renderer *world_renderer, size_t new_capacity)
{
  struct chunk_mesh *orphans = NULL;
  for(size_t i=0; i<world_renderer->chunk_mesh_capacity; ++i)
  {
    struct chunk_mesh **head = &world_renderer->chunk_meshes[i];
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

  world_renderer->chunk_meshes = realloc(world_renderer->chunk_meshes, new_capacity * sizeof *world_renderer->chunk_meshes);
  for(size_t i=world_renderer->chunk_mesh_capacity; i<new_capacity; ++i)
    world_renderer->chunk_meshes[i] = NULL;
  world_renderer->chunk_mesh_capacity = new_capacity;

  while(orphans)
  {
    struct chunk_mesh *orphan = orphans;
    orphans = orphans->next;

    orphan->next = world_renderer->chunk_meshes[orphan->hash % world_renderer->chunk_mesh_capacity];
    world_renderer->chunk_meshes[orphan->hash % world_renderer->chunk_mesh_capacity] = orphan;
  }
}

struct chunk_mesh *world_renderer_chunk_mesh_insert(struct world_renderer *world_renderer, int x, int y, int z)
{
  if(world_renderer->chunk_mesh_capacity == 0)
    world_renderer_chunk_mesh_rehash(world_renderer, 32);
  else if(world_renderer->chunk_mesh_load * 4 >= world_renderer->chunk_mesh_capacity * 3)
    world_renderer_chunk_mesh_rehash(world_renderer, world_renderer->chunk_mesh_capacity * 2);

  struct chunk_mesh *chunk_mesh = malloc(sizeof *chunk_mesh);
  chunk_mesh->x = x;
  chunk_mesh->y = y;
  chunk_mesh->z = z;
  chunk_mesh_init(chunk_mesh);

  chunk_mesh->hash = hash(x, y, z);
  chunk_mesh->next = world_renderer->chunk_meshes[chunk_mesh->hash % world_renderer->chunk_mesh_capacity];

  world_renderer->chunk_meshes[chunk_mesh->hash % world_renderer->chunk_mesh_capacity] = chunk_mesh;
  world_renderer->chunk_mesh_load += 1;

  return chunk_mesh;
}

struct chunk_mesh *world_renderer_chunk_mesh_lookup(struct world_renderer *world_renderer, int x, int y, int z)
{
  if(world_renderer->chunk_mesh_capacity == 0)
    return NULL;

  size_t h = hash(x, y, z);
  for(struct chunk_mesh *chunk_mesh = world_renderer->chunk_meshes[h % world_renderer->chunk_mesh_capacity]; chunk_mesh; chunk_mesh = chunk_mesh->next)
    if(chunk_mesh->hash == h && chunk_mesh->x == x && chunk_mesh->y == y && chunk_mesh->z == z)
      return chunk_mesh;
  return NULL;
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
        if(!(chunk = world_chunk_lookup(world, x+dx, y+dy, z+dz)))
          continue;

        if((chunk_mesh = world_renderer_chunk_mesh_lookup(world_renderer, x+dx, y+dy, z+dz)))
          continue;

        chunk_mesh = world_renderer_chunk_mesh_insert(world_renderer, x+dx, y+dy, z+dz);
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
  for(size_t i=0; i<world_renderer->chunk_mesh_capacity; ++i)
  {
    struct chunk_mesh **it = &world_renderer->chunk_meshes[i];
    while(*it)
      if((*it)->x < x - RENDERER_UNLOAD_DISTANCE || (*it)->x > x + RENDERER_UNLOAD_DISTANCE ||
         (*it)->y < y - RENDERER_UNLOAD_DISTANCE || (*it)->y > y + RENDERER_UNLOAD_DISTANCE ||
         (*it)->z < z - RENDERER_UNLOAD_DISTANCE || (*it)->z > z + RENDERER_UNLOAD_DISTANCE)
      {
        struct chunk_mesh *chunk_mesh = *it;
        *it = (*it)->next;

        chunk_mesh_deinit(chunk_mesh);
        free(chunk_mesh);
      }
      else
        it = &(*it)->next;
  }
}

void world_renderer_update_reload(struct world_renderer *world_renderer, struct world *world)
{
  struct chunk_mesh_builder chunk_mesh_builder;
  chunk_mesh_builder_init(&chunk_mesh_builder);

  struct chunk           *chunk;
  struct chunk_mesh      *chunk_mesh;
  struct chunk_adjacency  chunk_adjacency;

  for(size_t i=0; i<world_renderer->chunk_mesh_capacity; ++i)
    for(chunk_mesh = world_renderer->chunk_meshes[i]; chunk_mesh; chunk_mesh = chunk_mesh->next)
    {
      chunk = world_chunk_lookup(world, chunk_mesh->x, chunk_mesh->y, chunk_mesh->z);
      if(!chunk || !chunk->mesh_dirty)
        continue;

      chunk_adjacency_init(&chunk_adjacency, world, chunk);
      chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, &world_renderer->resource_pack, &chunk_adjacency);
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

  for(size_t i=0; i<world_renderer->chunk_mesh_capacity; ++i)
    for(struct chunk_mesh *chunk_mesh = world_renderer->chunk_meshes[i]; chunk_mesh; chunk_mesh = chunk_mesh->next)
    {
      glBindVertexArray(chunk_mesh->vao);
      glDrawElements(GL_TRIANGLES, chunk_mesh->count, GL_UNSIGNED_INT, 0);
    }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

