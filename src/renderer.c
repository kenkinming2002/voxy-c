#include <voxy/renderer.h>
#include <voxy/math.h>
#include <voxy/shader.h>
#include <voxy/cube_map.h>

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

#define ARRAY_LENGTH(arr) (sizeof (arr) / sizeof (arr)[0])

int renderer_init(struct renderer *renderer)
{
  renderer->skybox_program = 0;
  renderer->skybox_texture = 0;
  renderer->skybox_vao     = 0;
  renderer->skybox_vbo     = 0;
  renderer->skybox_ibo     = 0;

  renderer->chunk_program       = 0;
  renderer->chunk_meshes        = 0;
  renderer->chunk_mesh_capacity = 0;
  renderer->chunk_mesh_count    = 0;

  if((renderer->skybox_program = gl_program_load("assets/skybox.vert", "assets/skybox.frag")) == 0)
    goto error;

  if((renderer->skybox_texture = gl_cube_map_load((const char *[]){"assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg", "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg"})) == 0)
    goto error;

  struct vertex
  {
    struct vec3 position;
  };

  // FIXME: We do not need that much vertices since we do not have normals
  struct vertex vertices[ARRAY_LENGTH(CUBE_POSITIONS)];
  uint8_t       indices [ARRAY_LENGTH(CUBE_INDICES)];

  for(unsigned i=0; i<ARRAY_LENGTH(CUBE_POSITIONS); ++i)
    vertices[i].position = vec3_sub(vec3_mul(CUBE_POSITIONS[i], 2.0f), vec3(1.0f, 1.0f, 1.0f));

  for(unsigned i=0; i<ARRAY_LENGTH(CUBE_INDICES); ++i)
    indices[i] = CUBE_INDICES[i];

  glGenVertexArrays(1, &renderer->skybox_vao);
  glGenBuffers(1, &renderer->skybox_vbo);
  glGenBuffers(1, &renderer->skybox_ibo);

  glBindVertexArray(renderer->skybox_vao);

  glBindBuffer(GL_ARRAY_BUFFER, renderer->skybox_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderer->skybox_ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), 0);

  if((renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
    goto error;

  return 0;

error:
  if(renderer->skybox_program != 0) glDeleteProgram(renderer->skybox_program);
  if(renderer->skybox_texture != 0) glDeleteTextures(1, &renderer->skybox_texture);
  if(renderer->skybox_vao     != 0) glDeleteBuffers(1, &renderer->skybox_vao);
  if(renderer->skybox_vbo     != 0) glDeleteBuffers(1, &renderer->skybox_vbo);
  if(renderer->skybox_ibo     != 0) glDeleteBuffers(1, &renderer->skybox_ibo);

  if(renderer->chunk_program != 0) glDeleteProgram(renderer->chunk_program);

  return -1;
}

struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, struct chunk_mesh chunk_mesh)
{
  if(renderer->chunk_mesh_count == renderer->chunk_mesh_capacity)
  {
    renderer->chunk_mesh_capacity = renderer->chunk_mesh_count != 0 ? renderer->chunk_mesh_count * 2 : 1;
    renderer->chunk_meshes        = realloc(renderer->chunk_meshes, renderer->chunk_mesh_capacity * sizeof renderer->chunk_meshes[0]);
  }
  renderer->chunk_meshes[renderer->chunk_mesh_count++] = chunk_mesh;
  return &renderer->chunk_meshes[renderer->chunk_mesh_count-1];
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
    {
      struct chunk_mesh new_chunk_mesh;
      new_chunk_mesh.z = chunk->z;
      new_chunk_mesh.y = chunk->y;
      new_chunk_mesh.x = chunk->x;

      glGenVertexArrays(1, &new_chunk_mesh.vao);
      glGenBuffers(1, &new_chunk_mesh.vbo);
      glGenBuffers(1, &new_chunk_mesh.ibo);

      chunk_mesh = renderer_chunk_mesh_add(renderer, new_chunk_mesh);
    }

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
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  glEnable(GL_DEPTH_TEST);

  struct mat4 RP = mat4_identity();
  RP = mat4_mul(camera_rotation_matrix(camera),   RP);
  RP = mat4_mul(camera_projection_matrix(camera), RP);

  struct mat4 VP = mat4_identity();
  VP = mat4_mul(camera_view_matrix(camera),       VP);
  VP = mat4_mul(camera_projection_matrix(camera), VP);

  glDepthMask(GL_FALSE);
    glUseProgram(renderer->skybox_program);
    glUniformMatrix4fv(glGetUniformLocation(renderer->skybox_program, "RP"), 1, GL_TRUE, (const float *)&RP);
    glBindVertexArray(renderer->skybox_vao);
    glBindTexture(GL_TEXTURE_CUBE_MAP, renderer->skybox_texture);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_BYTE, 0);
  glDepthMask(GL_TRUE);

  glUseProgram(renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
  {
    glBindVertexArray(renderer->chunk_meshes[i].vao);
    glDrawElements(GL_TRIANGLES, renderer->chunk_meshes[i].count, GL_UNSIGNED_SHORT, 0);
  }
}

