#include <voxy/renderer.h>
#include <voxy/math.h>
#include <voxy/shader.h>

#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

int renderer_init(struct renderer *renderer)
{
  if((renderer->chunk_program = gl_program_load("assets/shader.vert",  "assets/shader.frag")) == 0)
    return -1;

  renderer->chunk_meshes        = 0;
  renderer->chunk_mesh_capacity = 0;
  renderer->chunk_mesh_count    = 0;
  return 0;
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

struct vertex
{
  struct vec3 position;
  struct vec3 color;
};

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

    struct vertex *vertices = malloc(24 * tile_count * sizeof *vertices);
    uint16_t      *indices  = malloc(36 * tile_count * sizeof *indices);

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

            vertices[24 * i + 0 ] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 1 ] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 2 ] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 3 ] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 4 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 5 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 6 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 7 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 8 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 9 ] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 10] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 11] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 12] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 13] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 14] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 15] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 16] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 17] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 18] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 19] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 0.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 20] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 21] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 0.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 22] = (struct vertex) { .position = vec3_add(anchor, vec3(0.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };
            vertices[24 * i + 23] = (struct vertex) { .position = vec3_add(anchor, vec3(1.0f, 1.0f, 1.0f)), .color = chunk->tiles[z][y][x].color, };

            indices[36 * i + 0]  = 24 * i + 0 ;
            indices[36 * i + 1]  = 24 * i + 1 ;
            indices[36 * i + 2]  = 24 * i + 2 ;
            indices[36 * i + 3]  = 24 * i + 2 ;
            indices[36 * i + 4]  = 24 * i + 1 ;
            indices[36 * i + 5]  = 24 * i + 3 ;
            indices[36 * i + 6]  = 24 * i + 4 ;
            indices[36 * i + 7]  = 24 * i + 5 ;
            indices[36 * i + 8]  = 24 * i + 6 ;
            indices[36 * i + 9]  = 24 * i + 6 ;
            indices[36 * i + 10] = 24 * i + 5 ;
            indices[36 * i + 11] = 24 * i + 7 ;
            indices[36 * i + 12] = 24 * i + 8 ;
            indices[36 * i + 13] = 24 * i + 9 ;
            indices[36 * i + 14] = 24 * i + 10;
            indices[36 * i + 15] = 24 * i + 10;
            indices[36 * i + 16] = 24 * i + 9 ;
            indices[36 * i + 17] = 24 * i + 11;
            indices[36 * i + 18] = 24 * i + 12;
            indices[36 * i + 19] = 24 * i + 13;
            indices[36 * i + 20] = 24 * i + 14;
            indices[36 * i + 21] = 24 * i + 14;
            indices[36 * i + 22] = 24 * i + 13;
            indices[36 * i + 23] = 24 * i + 15;
            indices[36 * i + 24] = 24 * i + 16;
            indices[36 * i + 25] = 24 * i + 17;
            indices[36 * i + 26] = 24 * i + 18;
            indices[36 * i + 27] = 24 * i + 18;
            indices[36 * i + 28] = 24 * i + 17;
            indices[36 * i + 29] = 24 * i + 19;
            indices[36 * i + 30] = 24 * i + 20;
            indices[36 * i + 31] = 24 * i + 21;
            indices[36 * i + 32] = 24 * i + 22;
            indices[36 * i + 33] = 24 * i + 22;
            indices[36 * i + 34] = 24 * i + 21;
            indices[36 * i + 35] = 24 * i + 23;

            ++i;
          }

    glBindVertexArray(chunk_mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, 24 * tile_count * sizeof *vertices, vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36  * tile_count * sizeof *indices, indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, position));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct vertex), (void *)offsetof(struct vertex, color));

    chunk_mesh->count = tile_count * 36;

    free(vertices);
    free(indices);
  }
}

void renderer_render(struct renderer *renderer, struct camera *camera)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glUseProgram(renderer->chunk_program);

  struct mat4 MVP = mat4_identity();
  MVP = mat4_mul(camera_view_matrix(camera),       MVP);
  MVP = mat4_mul(camera_projection_matrix(camera), MVP);

  glUniformMatrix4fv(glGetUniformLocation(renderer->chunk_program, "MVP"), 1, GL_TRUE, (const float *)&MVP);
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
  {
    glBindVertexArray(renderer->chunk_meshes[i].vao);
    glDrawElements(GL_TRIANGLES, renderer->chunk_meshes[i].count, GL_UNSIGNED_SHORT, 0);
  }
}

