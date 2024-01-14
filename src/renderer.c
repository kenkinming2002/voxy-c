#include <voxy/renderer.h>
#include <voxy/math.h>
#include <voxy/gl.h>

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/*******************
 * Chunk Adjacency *
 *******************/
void chunk_adjacency_init(struct chunk_adjacency *chunk_adjacency, struct world *world, struct chunk *chunk)
{
  chunk_adjacency->chunk = chunk;
  chunk_adjacency->left   = world_chunk_lookup(world, chunk->x-1, chunk->y, chunk->z);
  chunk_adjacency->right  = world_chunk_lookup(world, chunk->x+1, chunk->y, chunk->z);
  chunk_adjacency->back   = world_chunk_lookup(world, chunk->x, chunk->y-1, chunk->z);
  chunk_adjacency->front  = world_chunk_lookup(world, chunk->x, chunk->y+1, chunk->z);
  chunk_adjacency->bottom = world_chunk_lookup(world, chunk->x, chunk->y, chunk->z-1);
  chunk_adjacency->top    = world_chunk_lookup(world, chunk->x, chunk->y, chunk->z+1);
}

struct tile *chunk_adjacency_tile_lookup(struct chunk_adjacency *chunk_adjacency, int x, int y, int z)
{
  if(z >= 0 && z < CHUNK_WIDTH)
    if(y >= 0 && y < CHUNK_WIDTH)
      if(x >= 0 && x < CHUNK_WIDTH)
        return &chunk_adjacency->chunk->tiles[z][y][x];

  if(z == -1)          return chunk_adjacency->bottom ? &chunk_adjacency->bottom->tiles[CHUNK_WIDTH-1][y][x] : NULL;
  if(z == CHUNK_WIDTH) return chunk_adjacency->top    ? &chunk_adjacency->top   ->tiles[0]            [y][x] : NULL;

  if(y == -1)          return chunk_adjacency->back  ? &chunk_adjacency->back ->tiles[z][CHUNK_WIDTH-1][x] : NULL;
  if(y == CHUNK_WIDTH) return chunk_adjacency->front ? &chunk_adjacency->front->tiles[z][0]            [x] : NULL;

  if(x == -1)          return chunk_adjacency->left  ? &chunk_adjacency->left ->tiles[z][y][CHUNK_WIDTH-1] : NULL;
  if(x == CHUNK_WIDTH) return chunk_adjacency->right ? &chunk_adjacency->right->tiles[z][y][0]             : NULL;

  assert(0 && "Unreachable");
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

void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_adjacency *chunk_adjacency, int x, int y, int z, int dx, int dy, int dz)
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

      vertex.position = vec3_add  (vertex.position, vec3(chunk_adjacency->chunk->x, chunk_adjacency->chunk->y, chunk_adjacency->chunk->z));
      vertex.position = vec3_mul_s(vertex.position, CHUNK_WIDTH);
      vertex.position = vec3_add  (vertex.position, vec3(x, y, z));

      vertex.position = vec3_add(vertex.position, vec3(0.5f, 0.5f, 0.5f));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(normal, 0.5f));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(axis2, multipliers[i][0]));
      vertex.position = vec3_add(vertex.position, vec3_mul_s(axis1, multipliers[i][1]));

      switch(i)
      {
      case 0: vertex.texture_coords = vec2(0.0f, 0.0f); break;
      case 1: vertex.texture_coords = vec2(0.0f, 1.0f); break;
      case 2: vertex.texture_coords = vec2(1.0f, 0.0f); break;
      case 3: vertex.texture_coords = vec2(1.0f, 1.0f); break;
      }

      // FIXME: Unhardcode them
      switch(chunk_adjacency->chunk->tiles[z][y][x].id)
      {
      case TILE_ID_GRASS:
        switch(dz)
        {
        case -1: vertex.texture_index = 0; break;
        case  0: vertex.texture_index = 1; break;
        case  1: vertex.texture_index = 2; break;
        default:
          assert(0 && "Unreachable");
        }
        break;
      case TILE_ID_STONE:
        vertex.texture_index = 3;
        break;
      default:
        assert(0 && "Unreachable");
      }
    }
    chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertex);
  }
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

void chunk_mesh_update(struct chunk_mesh *chunk_mesh, struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_adjacency *chunk_adjacency)
{
  chunk_mesh_builder_reset(chunk_mesh_builder);
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
        if(chunk_adjacency->chunk->tiles[z][y][x].id != TILE_ID_EMPTY)
        {
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z, -1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z,  1,  0,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z,  0, -1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z,  0,  1,  0);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z,  0,  0, -1);
          chunk_mesh_builder_emit_face(chunk_mesh_builder, chunk_adjacency, x, y, z,  0,  0,  1);
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
  const char *CHUNK_VERTEX_SHADER_FILEPATH = "assets/chunk.vert";
  const char *CHUNK_FRAGMENT_SHADER_FILEPATH = "assets/chunk.frag";
  const char *CHUNK_BLOCK_TEXTURE_FILEPATHS[] = {
    "assets/grass_bottom.png",
    "assets/grass_side.png",
    "assets/grass_top.png",
    "assets/stone.png",
  };

  renderer->chunk_program             = 0;
  renderer->chunk_block_texture_array = 0;
  renderer->chunk_meshes              = NULL;
  renderer->chunk_mesh_count          = 0;
  renderer->chunk_mesh_capacity       = 0;

  if((renderer->chunk_program = gl_program_load(CHUNK_VERTEX_SHADER_FILEPATH, CHUNK_FRAGMENT_SHADER_FILEPATH)) == 0)
    goto error;

  if((renderer->chunk_block_texture_array = gl_array_texture_load(sizeof CHUNK_BLOCK_TEXTURE_FILEPATHS / sizeof CHUNK_BLOCK_TEXTURE_FILEPATHS[0], CHUNK_BLOCK_TEXTURE_FILEPATHS)) == 0)
    goto error;

  return 0;

error:
  if(renderer->chunk_program != 0)             glDeleteProgram(renderer->chunk_program);
  if(renderer->chunk_block_texture_array != 0) glDeleteTextures(1, &renderer->chunk_block_texture_array);
  return -1;
}

void renderer_deinit(struct renderer *renderer)
{
  glDeleteProgram(renderer->chunk_program);
  glDeleteTextures(1, &renderer->chunk_block_texture_array);
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
    chunk_mesh_deinit(&renderer->chunk_meshes[i]);
  free(renderer->chunk_meshes);
}

struct chunk_mesh *renderer_chunk_mesh_add(struct renderer *renderer, int x, int y, int z)
{
  if(renderer->chunk_mesh_count == renderer->chunk_mesh_capacity)
  {
    renderer->chunk_mesh_capacity = renderer->chunk_mesh_count != 0 ? renderer->chunk_mesh_count * 2 : 1;
    renderer->chunk_meshes        = realloc(renderer->chunk_meshes, renderer->chunk_mesh_capacity * sizeof renderer->chunk_meshes[0]);
  }
  struct chunk_mesh *chunk_mesh = &renderer->chunk_meshes[renderer->chunk_mesh_count++];
  chunk_mesh->x = x;
  chunk_mesh->y = y;
  chunk_mesh->z = z;
  chunk_mesh_init(chunk_mesh);
  return chunk_mesh;
}

struct chunk_mesh *renderer_chunk_mesh_lookup(struct renderer *renderer, int x, int y, int z)
{
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
  {
    if(renderer->chunk_meshes[i].x != x) continue;
    if(renderer->chunk_meshes[i].y != y) continue;
    if(renderer->chunk_meshes[i].z != z) continue;
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
      struct chunk_mesh *chunk_mesh = renderer_chunk_mesh_lookup(renderer, chunk->x, chunk->y, chunk->z);
      if(!chunk_mesh)
        chunk_mesh = renderer_chunk_mesh_add(renderer, chunk->x, chunk->y, chunk->z);

      struct chunk_adjacency chunk_adjacency;
      chunk_adjacency_init(&chunk_adjacency, world, chunk);
      chunk_mesh_update(chunk_mesh, &chunk_mesh_builder, &chunk_adjacency);
      chunk->remesh = false;
    }
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
  for(size_t i=0; i<renderer->chunk_mesh_count; ++i)
  {
    glBindVertexArray(renderer->chunk_meshes[i].vao);
    glDrawElements(GL_TRIANGLES, renderer->chunk_meshes[i].count, GL_UNSIGNED_INT, 0);
  }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

