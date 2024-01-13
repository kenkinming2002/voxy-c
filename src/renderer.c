#include <voxy/renderer.h>
#include <voxy/math.h>
#include <voxy/gl.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_LENGTH(arr) (sizeof (arr) / sizeof (arr)[0])

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

void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct chunk *chunk, int z, int y, int x, int dz, int dy, int dx)
{
  int nz = z + dz;
  int ny = y + dy;
  int nx = x + dx;
  if(nz >= 0 && nz < CHUNK_WIDTH)
    if(ny >= 0 && ny < CHUNK_WIDTH)
      if(nx >= 0 && nx < CHUNK_WIDTH)
        if(chunk->tiles[nz][ny][nx].present)
          return; // Occlusion

  // Fancy way to compute vertex positions without just dumping a big table here
  // Pray that the compiler will just inline everything (With -ffast-math probably)

  struct vec3 normal = vec3(dx, dy, dz);
  struct vec3 axis1  = vec3_dot(normal, vec3(0.0f, 0.0f, 1.0f)) == 0.0f ? vec3(0.0f, 0.0f, 1.0f) : vec3(1.0f, 0.0f, 0.0f);
  struct vec3 axis2  = vec3_cross(normal, axis1);

  const float multipliers[4][2] = {
    {-0.5f, -0.5f},
    {-0.5f,  0.5f},
    { 0.5f, -0.5f},
    { 0.5f,  0.5f},
  };

  for(unsigned i=0; i<4; ++i)
  {
    struct chunk_mesh_vertex vertex;
    {
      vertex.position = vec3_zero();

      vertex.position = vec3_add  (vertex.position, vec3(chunk->x, chunk->y, chunk->z));
      vertex.position = vec3_mul_s(vertex.position, CHUNK_WIDTH);
      vertex.position = vec3_add  (vertex.position, vec3(x, y, z));

      vertex.position = vec3_add(vertex.position, vec3(0.5f, 0.5f, 0.5f));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(normal, 0.5f));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(axis1, multipliers[i][0]));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(axis2, multipliers[i][1]));

      vertex.color = chunk->tiles[z][y][x].color;
    }
    chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertex);
  }

  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 0);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 1);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 2);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 2);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 1);
  chunk_mesh_builder_push_index(chunk_mesh_builder, chunk_mesh_builder->vertex_count + 3);
}


/***************
 * Chunk Mesh *
 **************/
int chunk_mesh_init(struct chunk_mesh *chunk_mesh)
{
  glGenVertexArrays(1, &chunk_mesh->vao);
  glGenBuffers(1, &chunk_mesh->vbo);
  glGenBuffers(1, &chunk_mesh->ibo);
  return 0;
}

void chunk_mesh_deinit(struct chunk_mesh *chunk_mesh)
{
  glDeleteVertexArrays(1, &chunk_mesh->vao);
  glDeleteBuffers(1, &chunk_mesh->vbo);
  glDeleteBuffers(1, &chunk_mesh->ibo);
}

void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct chunk *chunk)
{
  chunk_mesh_builder_reset(chunk_mesh_builder);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk->tiles[z][y][x].present)
        {
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x, -1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x,  1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x,  0, -1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x,  0,  1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x,  0,  0, -1);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk, z, y, x,  0,  0,  1);
        }

  glBindVertexArray(chunk_mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, chunk_mesh_builder->vertex_count * sizeof *chunk_mesh_builder->vertices, chunk_mesh_builder->vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_builder->index_count * sizeof *chunk_mesh_builder->indices, chunk_mesh_builder->indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, color));

  chunk_mesh->count = chunk_mesh_builder->index_count;
}

/******************
 * Chunk Renderer *
 ******************/
int chunk_renderer_init(struct chunk_renderer *chunk_renderer)
{
  if((chunk_renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
    goto error;

  return 0;

error:
  return -1;
}

void chunk_renderer_deinit(struct chunk_renderer *chunk_renderer)
{
  glDeleteProgram(chunk_renderer->chunk_program);
}

void chunk_renderer_begin(struct chunk_renderer *chunk_renderer, struct camera *camera)
{
  struct mat4 VP = mat4_identity();
  VP = mat4_mul(camera_view_matrix(camera),       VP);
  VP = mat4_mul(camera_projection_matrix(camera), VP);

  glUseProgram(chunk_renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(chunk_renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
}

void chunk_renderer_render(struct chunk_renderer *chunk_renderer, struct chunk_mesh *chunk_mesh)
{
  (void)chunk_renderer;

  glBindVertexArray(chunk_mesh->vao);
  glDrawElements(GL_TRIANGLES, chunk_mesh->count, GL_UNSIGNED_INT, 0);
}

/************
 * Renderer *
 ************/
int renderer_init(struct renderer *renderer)
{
  bool chunk_renderer_initialized  = false;
  if(chunk_renderer_init(&renderer->chunk_renderer) != 0)
    goto error;
  chunk_renderer_initialized = true;

  renderer->chunk_meshes        = NULL;
  renderer->chunk_mesh_capacity = 0;
  renderer->chunk_mesh_count    = 0;
  return 0;

error:
  if(chunk_renderer_initialized)
    chunk_renderer_deinit(&renderer->chunk_renderer);

  return -1;
}

void renderer_deinit(struct renderer *renderer)
{
  chunk_renderer_deinit(&renderer->chunk_renderer);
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
    chunk_mesh_deinit(&renderer->chunk_meshes[i]);
  free(renderer->chunk_meshes);
}

struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, int z, int y, int x)
{
  if(renderer->chunk_mesh_count == renderer->chunk_mesh_capacity)
  {
    renderer->chunk_mesh_capacity = renderer->chunk_mesh_count != 0 ? renderer->chunk_mesh_count * 2 : 1;
    renderer->chunk_meshes        = realloc(renderer->chunk_meshes, renderer->chunk_mesh_capacity * sizeof renderer->chunk_meshes[0]);
  }
  struct chunk_mesh *chunk_mesh = &renderer->chunk_meshes[renderer->chunk_mesh_count++];
  chunk_mesh->z = z;
  chunk_mesh->y = y;
  chunk_mesh->x = x;
  chunk_mesh_init(chunk_mesh);
  return chunk_mesh;
}

struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int z, int y, int x)
{
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
  {
    if(renderer->chunk_meshes[i].z != z) continue;
    if(renderer->chunk_meshes[i].y != y) continue;
    if(renderer->chunk_meshes[i].x != x) continue;
    return &renderer->chunk_meshes[i];
  }
  return NULL;
}

void renderer_update(struct renderer *renderer, struct world *world)
{
  struct chunk_mesh_builder chunk_mesh_builder;
  chunk_mesh_builder_init(&chunk_mesh_builder);
  for(size_t i=0; i<world->chunk_count; ++i)
  {
    struct chunk *chunk = &world->chunks[i];
    if(chunk->remesh)
    {
      struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup(renderer, chunk->z, chunk->y, chunk->x);
      if(!chunk_mesh)
        chunk_mesh = renderer_chunk_mesh_add(renderer, chunk->z, chunk->y, chunk->x);

      chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, chunk);
      chunk->remesh = false;
    }
  }
  chunk_mesh_builder_deinit(&chunk_mesh_builder);
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  glEnable(GL_DEPTH_TEST);
    chunk_renderer_begin(&renderer->chunk_renderer, camera);
    for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
      chunk_renderer_render(&renderer->chunk_renderer, &renderer->chunk_meshes[i]);
  glDisable(GL_DEPTH_TEST);
}

