#include <voxy/renderer.h>
#include <voxy/math.h>
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

  struct vertex *vertices = malloc(tile_count * ARRAY_LENGTH(CUBE_POSITIONS) * sizeof *vertices);
  uint16_t      *indices  = malloc(tile_count * ARRAY_LENGTH(CUBE_INDICES)   * sizeof *indices);

  unsigned i = 0;
  for(unsigned z = 0; z<CHUNK_WIDTH; ++z)
    for(unsigned y = 0; y<CHUNK_WIDTH; ++y)
      for(unsigned x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk->tiles[z][y][x].present)
        {
          struct vec3 anchor = vec3_zero();
          anchor = vec3_add(anchor, vec3(chunk->x, chunk->y, chunk->z));
          anchor = vec3_mul(anchor, 16.0f);
          anchor = vec3_add(anchor, vec3(x, y, z));

          for(unsigned j=0; j<ARRAY_LENGTH(CUBE_POSITIONS); ++j)
          {
            vertices[i * ARRAY_LENGTH(CUBE_POSITIONS) + j].position = vec3_add(anchor, CUBE_POSITIONS[j]);
            vertices[i * ARRAY_LENGTH(CUBE_POSITIONS) + j].color    = chunk->tiles[z][y][x].color;
          }

          for(unsigned j=0; j<ARRAY_LENGTH(CUBE_INDICES); ++j)
            indices[i * ARRAY_LENGTH(CUBE_INDICES) + j] = i * ARRAY_LENGTH(CUBE_POSITIONS) + CUBE_INDICES[j];

          ++i;
        }

  glBindVertexArray(chunk_mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
  glBufferData(GL_ARRAY_BUFFER, tile_count * ARRAY_LENGTH(CUBE_POSITIONS) * sizeof *vertices, vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, tile_count * ARRAY_LENGTH(CUBE_INDICES) * sizeof *indices, indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, position));
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, color));

  chunk_mesh->count = tile_count * ARRAY_LENGTH(CUBE_INDICES);

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
  glDrawElements(GL_TRIANGLES, chunk_mesh->count, GL_UNSIGNED_SHORT, 0);
}

/**********
 * Skybox *
 **********/
int skybox_load(struct skybox *skybox, const char *filepaths[6])
{
  if((skybox->skybox_texture = gl_cube_map_texture_load(filepaths)) == 0)
    goto error;

  return 0;

error:
  return -1;
}

void skybox_unload(struct skybox *skybox)
{
  glDeleteTextures(1, &skybox->skybox_texture);
}

/*******************
 * Skybox Renderer *
 *******************/
int skybox_renderer_init(struct skybox_renderer *skybox_renderer)
{
  skybox_renderer->skybox_program = 0;
  skybox_renderer->skybox_vao     = 0;
  skybox_renderer->skybox_vbo     = 0;
  skybox_renderer->skybox_ibo     = 0;

  if((skybox_renderer->skybox_program = gl_program_load("assets/skybox.vert", "assets/skybox.frag")) == 0)
    goto error;

  // FIXME: We do not need that much vertices since we do not have normals
  struct vec3 positions[ARRAY_LENGTH(CUBE_POSITIONS)];
  uint8_t     indices  [ARRAY_LENGTH(CUBE_INDICES)];

  for(unsigned i=0; i<ARRAY_LENGTH(CUBE_POSITIONS); ++i)
    positions[i] = vec3_sub(vec3_mul(CUBE_POSITIONS[i], 2.0f), vec3(1.0f, 1.0f, 1.0f));

  for(unsigned i=0; i<ARRAY_LENGTH(CUBE_INDICES); ++i)
    indices[i] = CUBE_INDICES[i];

  glGenVertexArrays(1, &skybox_renderer->skybox_vao);
  glGenBuffers(1, &skybox_renderer->skybox_vbo);
  glGenBuffers(1, &skybox_renderer->skybox_ibo);

  glBindVertexArray(skybox_renderer->skybox_vao);

  glBindBuffer(GL_ARRAY_BUFFER, skybox_renderer->skybox_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof positions, positions, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skybox_renderer->skybox_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vec3), 0);

  return 0;

error:
  if(skybox_renderer->skybox_program != 0) glDeleteProgram(skybox_renderer->skybox_program);
  if(skybox_renderer->skybox_vao     != 0) glDeleteBuffers(1, &skybox_renderer->skybox_vao);
  if(skybox_renderer->skybox_vbo     != 0) glDeleteBuffers(1, &skybox_renderer->skybox_vbo);
  if(skybox_renderer->skybox_ibo     != 0) glDeleteBuffers(1, &skybox_renderer->skybox_ibo);
  return -1;
}

void skybox_renderer_deinit(struct skybox_renderer *skybox_renderer)
{
  glDeleteProgram(skybox_renderer->skybox_program);
  glDeleteBuffers(1, &skybox_renderer->skybox_vao);
  glDeleteBuffers(1, &skybox_renderer->skybox_vbo);
  glDeleteBuffers(1, &skybox_renderer->skybox_ibo);
}

void skybox_renderer_render(struct skybox_renderer *skybox_renderer, struct camera *camera, struct skybox *skybox)
{
  struct mat4 RP = mat4_identity();
  RP = mat4_mul(camera_rotation_matrix(camera),   RP);
  RP = mat4_mul(camera_projection_matrix(camera), RP);

  glDepthMask(GL_FALSE);
    glUseProgram(skybox_renderer->skybox_program);
    glUniformMatrix4fv(glGetUniformLocation(skybox_renderer->skybox_program, "RP"), 1, GL_TRUE, (const float *)&RP);
    glBindVertexArray(skybox_renderer->skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->skybox_texture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
  glDepthMask(GL_TRUE);
}

/************
 * Renderer *
 ************/
int renderer_init(struct renderer *renderer)
{
  bool skybox_renderer_initialized = false;
  bool skybox_loaded               = false;
  bool chunk_renderer_initialized  = false;

  if(skybox_renderer_init(&renderer->skybox_renderer) != 0)
    goto error;
  skybox_renderer_initialized = true;

  if(skybox_load(&renderer->skybox, (const char *[]){"assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg", "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg"}) != 0)
    goto error;
  skybox_loaded = true;

  if(chunk_renderer_init(&renderer->chunk_renderer) != 0)
    goto error;
  chunk_renderer_initialized = true;

  renderer->chunk_meshes        = NULL;
  renderer->chunk_mesh_capacity = 0;
  renderer->chunk_mesh_count    = 0;
  return 0;

error:
  if(skybox_renderer_initialized)
    skybox_renderer_deinit(&renderer->skybox_renderer);

  if(skybox_loaded)
    skybox_unload(&renderer->skybox);

  if(chunk_renderer_initialized)
    chunk_renderer_deinit(&renderer->chunk_renderer);

  return -1;
}

void renderer_deinit(struct renderer *renderer)
{
  skybox_renderer_deinit(&renderer->skybox_renderer);
  skybox_unload(&renderer->skybox);
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

    struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup(renderer, chunk->z, chunk->y, chunk->x);
    if(!chunk_mesh)
      chunk_mesh = renderer_chunk_mesh_add(renderer, chunk->z, chunk->y, chunk->x);

    chunk_mesh_update(chunk_mesh, chunk);
  }
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  glEnable(GL_DEPTH_TEST);

  skybox_renderer_render(&renderer->skybox_renderer, camera, &renderer->skybox);
  chunk_renderer_begin(&renderer->chunk_renderer, camera);
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
    chunk_renderer_render(&renderer->chunk_renderer, &renderer->chunk_meshes[i]);
}

