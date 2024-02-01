#include "world_renderer.h"

#include <voxy/math/vector.h>

#include "gl.h"
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int world_renderer_init(struct world_renderer *world_renderer, struct resource_pack *resource_pack)
{
  world_renderer->chunk_program             = 0;
  world_renderer->chunk_block_texture_array = 0;

  if((world_renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load chunk shader\n");
    goto error;
  }

  size_t       filepath_count = resource_pack->block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);

  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;
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

  return -1;
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
  glDeleteProgram(world_renderer->chunk_program);
  glDeleteTextures(1, &world_renderer->chunk_block_texture_array);
}

struct chunk_mesh_vertex
{
  fvec3_t  position;
  fvec2_t  texture_coords;
  uint32_t texture_index;
  float    light_level;
};

struct chunk_mesh_info
{
  struct chunk *chunk;

  struct chunk_mesh_vertex *vertices;
  size_t                    vertex_count;
  size_t                    vertex_capacity;

  uint32_t *indices;
  size_t    index_count;
  size_t    index_capacity;
};

__attribute__((always_inline))
static inline struct tile *chunk_mesh_info_tile_lookup(struct chunk_mesh_info *chunk_mesh_info, ivec3_t cposition)
{
  if(cposition.z >= 0 && cposition.z < CHUNK_WIDTH)
    if(cposition.y >= 0 && cposition.y < CHUNK_WIDTH)
      if(cposition.x >= 0 && cposition.x < CHUNK_WIDTH)
        return &chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x];

  if(cposition.z == -1)          return chunk_mesh_info->chunk->bottom ? &chunk_mesh_info->chunk->bottom->chunk_data->tiles[CHUNK_WIDTH-1][cposition.y][cposition.x] : NULL;
  if(cposition.z == CHUNK_WIDTH) return chunk_mesh_info->chunk->top    ? &chunk_mesh_info->chunk->top   ->chunk_data->tiles[0]            [cposition.y][cposition.x] : NULL;

  if(cposition.y == -1)          return chunk_mesh_info->chunk->back  ? &chunk_mesh_info->chunk->back ->chunk_data->tiles[cposition.z][CHUNK_WIDTH-1][cposition.x] : NULL;
  if(cposition.y == CHUNK_WIDTH) return chunk_mesh_info->chunk->front ? &chunk_mesh_info->chunk->front->chunk_data->tiles[cposition.z][0]            [cposition.x] : NULL;

  if(cposition.x == -1)          return chunk_mesh_info->chunk->left  ? &chunk_mesh_info->chunk->left ->chunk_data->tiles[cposition.z][cposition.y][CHUNK_WIDTH-1] : NULL;
  if(cposition.x == CHUNK_WIDTH) return chunk_mesh_info->chunk->right ? &chunk_mesh_info->chunk->right->chunk_data->tiles[cposition.z][cposition.y][0]             : NULL;

  assert(0 && "Unreachable");
}

__attribute__((always_inline))
static inline void chunk_mesh_info_push_vertex(struct chunk_mesh_info *chunk_mesh_info, struct chunk_mesh_vertex vertex)
{
  if(chunk_mesh_info->vertex_capacity == chunk_mesh_info->vertex_count)
  {
    chunk_mesh_info->vertex_capacity = chunk_mesh_info->vertex_capacity != 0 ? chunk_mesh_info->vertex_capacity * 2 : 1;
    chunk_mesh_info->vertices        = realloc(chunk_mesh_info->vertices, chunk_mesh_info->vertex_capacity * sizeof *chunk_mesh_info->vertices);
  }
  chunk_mesh_info->vertices[chunk_mesh_info->vertex_count++] = vertex;
}

__attribute__((always_inline))
static inline void chunk_mesh_info_push_index(struct chunk_mesh_info *chunk_mesh_info, uint32_t index)
{
  if(chunk_mesh_info->index_capacity == chunk_mesh_info->index_count)
  {
    chunk_mesh_info->index_capacity = chunk_mesh_info->index_capacity != 0 ? chunk_mesh_info->index_capacity * 2 : 1;
    chunk_mesh_info->indices        = realloc(chunk_mesh_info->indices, chunk_mesh_info->index_capacity * sizeof *chunk_mesh_info->indices);
  }
  chunk_mesh_info->indices[chunk_mesh_info->index_count++] = index;
}

__attribute__((always_inline))
static inline void chunk_mesh_info_emit_face(struct chunk_mesh_info *chunk_mesh_info, struct resource_pack *resource_pack, ivec3_t cposition, ivec3_t dcposition)
{
  struct tile *ntile = chunk_mesh_info_tile_lookup(chunk_mesh_info, ivec3_add(cposition, dcposition));
  if(!ntile || resource_pack->block_infos[ntile->id].type == BLOCK_TYPE_INVISIBLE)
  {
    //////////////////
    /// 1: Indices ///
    //////////////////
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 0);
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 1);
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 2);
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 2);
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 1);
    chunk_mesh_info_push_index(chunk_mesh_info, chunk_mesh_info->vertex_count + 3);

    ///////////////////
    /// 2: Vertices ///
    ///////////////////
    fvec3_t normal = fvec3(dcposition.x, dcposition.y, dcposition.z);
    fvec3_t axis2  = fvec3_dot(normal, fvec3(0.0f, 0.0f, 1.0f)) == 0.0f ? fvec3(0.0f, 0.0f, 1.0f) : fvec3(1.0f, 0.0f, 0.0f);
    fvec3_t axis1  = fvec3_cross(normal, axis2);

    struct chunk_mesh_vertex vertices[4];

    fvec3_t center = fvec3_zero();;
    center = fvec3_add       (center, fvec3(chunk_mesh_info->chunk->position.x, chunk_mesh_info->chunk->position.y, chunk_mesh_info->chunk->position.z));
    center = fvec3_mul_scalar(center, CHUNK_WIDTH);
    center = fvec3_add       (center, fvec3(cposition.x, cposition.y, cposition.z));
    center = fvec3_add       (center, fvec3_mul_scalar(normal, 0.5f));

    vertices[0].position = fvec3_add(center, fvec3_add(fvec3_mul_scalar(axis1, -0.5f), fvec3_mul_scalar(axis2, -0.5f)));
    vertices[1].position = fvec3_add(center, fvec3_add(fvec3_mul_scalar(axis1, -0.5f), fvec3_mul_scalar(axis2,  0.5f)));
    vertices[2].position = fvec3_add(center, fvec3_add(fvec3_mul_scalar(axis1,  0.5f), fvec3_mul_scalar(axis2, -0.5f)));
    vertices[3].position = fvec3_add(center, fvec3_add(fvec3_mul_scalar(axis1,  0.5f), fvec3_mul_scalar(axis2,  0.5f)));

    vertices[0].texture_coords = fvec2(0.0f, 0.0f);
    vertices[1].texture_coords = fvec2(0.0f, 1.0f);
    vertices[2].texture_coords = fvec2(1.0f, 0.0f);
    vertices[3].texture_coords = fvec2(1.0f, 1.0f);

    uint32_t texture_index;
    if     (dcposition.x == -1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_left;
    else if(dcposition.x ==  1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_right;
    else if(dcposition.y == -1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_back;
    else if(dcposition.y ==  1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_front;
    else if(dcposition.z == -1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_bottom;
    else if(dcposition.z ==  1) texture_index = resource_pack->block_infos[chunk_mesh_info->chunk->chunk_data->tiles[cposition.z][cposition.y][cposition.x].id].texture_top;
    else
      assert(0 && "Unreachable");

    vertices[0].texture_index = texture_index;
    vertices[1].texture_index = texture_index;
    vertices[2].texture_index = texture_index;
    vertices[3].texture_index = texture_index;

    vertices[0].light_level = ntile ? ntile->light_level / 15.0f : 0.0f;
    vertices[1].light_level = ntile ? ntile->light_level / 15.0f : 0.0f;
    vertices[2].light_level = ntile ? ntile->light_level / 15.0f : 0.0f;
    vertices[3].light_level = ntile ? ntile->light_level / 15.0f : 0.0f;

    chunk_mesh_info_push_vertex(chunk_mesh_info, vertices[0]);
    chunk_mesh_info_push_vertex(chunk_mesh_info, vertices[1]);
    chunk_mesh_info_push_vertex(chunk_mesh_info, vertices[2]);
    chunk_mesh_info_push_vertex(chunk_mesh_info, vertices[3]);
  }
}

void world_renderer_update(struct world_renderer *world_renderer, struct resource_pack *resource_pack, struct world *world)
{
  struct chunk_mesh_info *chunk_mesh_infos         = NULL;
  size_t                  chunk_mesh_info_count    = 0;
  size_t                  chunk_mesh_info_capacity = 0;

  ivec3_t player_chunk_position = fvec3_as_ivec3_floor(fvec3_div_scalar(world->player.transform.translation, CHUNK_WIDTH));

  ////////////////////////////////////////////////////
  /// 1: Collect all chunks that need to be meshed ///
  ////////////////////////////////////////////////////
  for(int dz = -RENDERER_LOAD_DISTANCE; dz<=RENDERER_LOAD_DISTANCE; ++dz)
    for(int dy = -RENDERER_LOAD_DISTANCE; dy<=RENDERER_LOAD_DISTANCE; ++dy)
      for(int dx = -RENDERER_LOAD_DISTANCE; dx<=RENDERER_LOAD_DISTANCE; ++dx)
      {
        ivec3_t chunk_position = ivec3_add(player_chunk_position, ivec3(dx, dy, dz));
        struct chunk *chunk = chunk_hash_table_lookup(&world->chunks, chunk_position);
        if(chunk && chunk->mesh_dirty)
        {
          struct chunk_mesh_info chunk_mesh_info;

          chunk_mesh_info.chunk  = chunk;

          chunk_mesh_info.vertices        = NULL;
          chunk_mesh_info.vertex_count    = 0;
          chunk_mesh_info.vertex_capacity = 0;

          chunk_mesh_info.indices        = NULL;
          chunk_mesh_info.index_count    = 0;
          chunk_mesh_info.index_capacity = 0;

          if(chunk_mesh_info_capacity == chunk_mesh_info_count)
          {
            chunk_mesh_info_capacity = chunk_mesh_info_capacity != 0 ? chunk_mesh_info_capacity * 2 : 1;
            chunk_mesh_infos         = realloc(chunk_mesh_infos, chunk_mesh_info_capacity * sizeof *chunk_mesh_infos);
          }
          chunk_mesh_infos[chunk_mesh_info_count++] = chunk_mesh_info;
        }
      }

  //////////////////////////////////////
  /// 2: Mesh all chunks in parallel ///
  //////////////////////////////////////
  #pragma omp parallel for
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
  {
    chunk_mesh_infos[i].chunk->mesh_dirty = false;
    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
          if(resource_pack->block_infos[chunk_mesh_infos[i].chunk->chunk_data->tiles[z][y][x].id].type != BLOCK_TYPE_INVISIBLE)
          {
            ivec3_t position = ivec3(x, y, z);
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3(-1,  0,  0));
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3( 1,  0,  0));
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3( 0, -1,  0));
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3( 0,  1,  0));
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3( 0,  0, -1));
            chunk_mesh_info_emit_face(&chunk_mesh_infos[i], resource_pack, position, ivec3( 0,  0,  1));
          }
  }

  //////////////////////////////
  /// 3: Upload chunk meshes ///
  //////////////////////////////
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
  {
    struct chunk_mesh *chunk_mesh = chunk_mesh_infos[i].chunk->chunk_mesh;
    if(!chunk_mesh)
    {
      chunk_mesh = malloc(sizeof *chunk_mesh);

      glGenVertexArrays(1, &chunk_mesh->vao);
      glGenBuffers(1, &chunk_mesh->vbo);
      glGenBuffers(1, &chunk_mesh->ibo);

      chunk_mesh_infos[i].chunk->chunk_mesh = chunk_mesh;
    }

    glBindVertexArray(chunk_mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh_infos[i].vertex_count * sizeof *chunk_mesh_infos[i].vertices, chunk_mesh_infos[i].vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_infos[i].index_count * sizeof *chunk_mesh_infos[i].indices, chunk_mesh_infos[i].indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, light_level));

    chunk_mesh->count = chunk_mesh_infos[i].index_count;
  }

  //////////////////////////////
  /// 4: Unload chunk meshes ///
  //////////////////////////////
  for(size_t i=0; i<world->chunks.bucket_count; ++i)
    for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
      if(chunk->chunk_mesh)
        if(chunk->position.x < player_chunk_position.x - RENDERER_UNLOAD_DISTANCE || chunk->position.x > player_chunk_position.x + RENDERER_UNLOAD_DISTANCE ||
           chunk->position.y < player_chunk_position.y - RENDERER_UNLOAD_DISTANCE || chunk->position.y > player_chunk_position.y + RENDERER_UNLOAD_DISTANCE ||
           chunk->position.z < player_chunk_position.z - RENDERER_UNLOAD_DISTANCE || chunk->position.z > player_chunk_position.z + RENDERER_UNLOAD_DISTANCE)
        {
          glDeleteVertexArrays(1, &chunk->chunk_mesh->vao);
          glDeleteBuffers(1, &chunk->chunk_mesh->vbo);
          glDeleteBuffers(1, &chunk->chunk_mesh->ibo);
          free(chunk->chunk_mesh);

          chunk->chunk_mesh = NULL;
        }

  //////////////////
  /// 5: Cleanup ///
  //////////////////
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
  {
    free(chunk_mesh_infos[i].vertices);
    free(chunk_mesh_infos[i].indices);
  }
  free(chunk_mesh_infos);
}

void world_renderer_render(struct world_renderer *world_renderer, struct world *world, struct camera *camera)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(world_renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_CUBE_MAP, world_renderer->chunk_block_texture_array);

  for(size_t i=0; i<world->chunks.bucket_count; ++i)
    for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
      if(chunk->chunk_mesh)
      {
        glBindVertexArray(chunk->chunk_mesh->vao);
        glDrawElements(GL_TRIANGLES, chunk->chunk_mesh->count, GL_UNSIGNED_INT, 0);
      }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

