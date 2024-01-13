#include <voxy/renderer.h>
#include <voxy/math.h>
#include <voxy/cube.h>
#include <voxy/gl.h>

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_LENGTH(arr) (sizeof (arr) / sizeof (arr)[0])

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

void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk *chunk)
{
  unsigned tile_count = 0;
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk->tiles[z][y][x].present)
          ++tile_count;

  struct vertex
  {
    struct vec3 position;
    struct vec3 color;
  };

  struct vertex *vertices = malloc(tile_count * 24 * sizeof *vertices);
  uint32_t      *indices  = malloc(tile_count * 36   * sizeof *indices);

  unsigned i = 0;
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk->tiles[z][y][x].present)
        {
          struct vec3 center = vec3_zero();
          center = vec3_add(center, vec3(chunk->x, chunk->y, chunk->z));
          center = vec3_mul_s(center, CHUNK_WIDTH);
          center = vec3_add(center, vec3(x, y, z));

          struct vec3 cube_positions[24];
          struct vec3 cube_normals[24];
          uint32_t    cube_indices[36];
          cube_emit(center, 24 * i, cube_positions, cube_normals, cube_indices);

          for(unsigned j=0; j<24; ++j)
          {
            vertices[24 * i + j].position = cube_positions[j];
            vertices[24 * i + j].color    = chunk->tiles[z][y][x].color;
          }

          for(unsigned j=0; j<36; ++j)
            indices[36 * i + j] = cube_indices[j];

          ++i;
        }

  glBindVertexArray(chunk_mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, tile_count * 24 * sizeof *vertices, vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tile_count * 36 * sizeof *indices, indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, color));

  chunk_mesh->count = tile_count * 36;

  free(vertices);
  free(indices);
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
  for(size_t i=0; i<world->chunk_count; ++i)
  {
    struct chunk *chunk = &world->chunks[i];
    if(!chunk->remesh)
      continue;
    chunk->remesh = false;

    struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup(renderer, chunk->z, chunk->y, chunk->x);
    if(!chunk_mesh)
      chunk_mesh = renderer_chunk_mesh_add(renderer, chunk->z, chunk->y, chunk->x);

    chunk_mesh_update(chunk_mesh, chunk);
  }
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  glEnable(GL_DEPTH_TEST);
    chunk_renderer_begin(&renderer->chunk_renderer, camera);
    for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
      chunk_renderer_render(&renderer->chunk_renderer, &renderer->chunk_meshes[i]);
  glDisable(GL_DEPTH_TEST);
}

