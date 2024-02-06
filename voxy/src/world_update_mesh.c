#include "world.h"

#include "resource_pack.h"

#include <stdlib.h>

struct chunk_mesh_vertex
{
  fvec3_t  position;
  fvec2_t  texture_coords;
  uint32_t texture_index;
  float    light_level;
};

struct chunk_mesh_builder
{
  struct chunk_mesh_vertex *vertices;
  size_t                    vertex_count;
  size_t                    vertex_capacity;

  uint32_t *indices;
  size_t    index_count;
  size_t    index_capacity;
};

static inline void chunk_mesh_builder_push_vertices(struct chunk_mesh_builder *chunk_mesh_builder, size_t count, struct chunk_mesh_vertex vertices[count])
{
  if(chunk_mesh_builder->vertex_capacity < chunk_mesh_builder->vertex_count + count)
  {
    while(chunk_mesh_builder->vertex_capacity < chunk_mesh_builder->vertex_count + count)
      chunk_mesh_builder->vertex_capacity = chunk_mesh_builder->vertex_capacity != 0 ? chunk_mesh_builder->vertex_capacity * 2 : 1;

    chunk_mesh_builder->vertices = realloc(chunk_mesh_builder->vertices, chunk_mesh_builder->vertex_capacity * sizeof *chunk_mesh_builder->vertices);
  }

  for(size_t i=0; i<count; ++i)
    chunk_mesh_builder->vertices[chunk_mesh_builder->vertex_count++] = vertices[i];
}

static inline void chunk_mesh_builder_push_indices(struct chunk_mesh_builder *chunk_mesh_builder, size_t count, uint32_t indices[count])
{
  if(chunk_mesh_builder->index_capacity < chunk_mesh_builder->index_count + count)
  {
    while(chunk_mesh_builder->index_capacity < chunk_mesh_builder->index_count + count)
      chunk_mesh_builder->index_capacity = chunk_mesh_builder->index_capacity != 0 ? chunk_mesh_builder->index_capacity * 2 : 1;

    chunk_mesh_builder->indices = realloc(chunk_mesh_builder->indices, chunk_mesh_builder->index_capacity * sizeof *chunk_mesh_builder->indices);
  }

  for(size_t i=0; i<count; ++i)
    chunk_mesh_builder->indices[chunk_mesh_builder->index_count++] = indices[i];
}

static inline void chunk_mesh_builder_emit_face(struct chunk_mesh_builder *chunk_mesh_builder, fvec3_t position, fvec3_t normal, uint32_t texture_index, float light_level)
{
  uint32_t                 indices[6];
  struct chunk_mesh_vertex vertices[4];

  indices[0] = chunk_mesh_builder->vertex_count + 0;
  indices[1] = chunk_mesh_builder->vertex_count + 1;
  indices[2] = chunk_mesh_builder->vertex_count + 2;
  indices[3] = chunk_mesh_builder->vertex_count + 2;
  indices[4] = chunk_mesh_builder->vertex_count + 1;
  indices[5] = chunk_mesh_builder->vertex_count + 3;

  fvec3_t axis2 = fvec3_dot(normal, fvec3(0.0f, 0.0f, 1.0f)) == 0.0f ? fvec3(0.0f, 0.0f, 1.0f) : fvec3(1.0f, 0.0f, 0.0f);
  fvec3_t axis1 = fvec3_cross(normal, axis2);

  vertices[0].position = fvec3_add(position, fvec3_add(fvec3_mul_scalar(normal, 0.5f), fvec3_add(fvec3_mul_scalar(axis1, -0.5f), fvec3_mul_scalar(axis2, -0.5f))));
  vertices[1].position = fvec3_add(position, fvec3_add(fvec3_mul_scalar(normal, 0.5f), fvec3_add(fvec3_mul_scalar(axis1, -0.5f), fvec3_mul_scalar(axis2,  0.5f))));
  vertices[2].position = fvec3_add(position, fvec3_add(fvec3_mul_scalar(normal, 0.5f), fvec3_add(fvec3_mul_scalar(axis1,  0.5f), fvec3_mul_scalar(axis2, -0.5f))));
  vertices[3].position = fvec3_add(position, fvec3_add(fvec3_mul_scalar(normal, 0.5f), fvec3_add(fvec3_mul_scalar(axis1,  0.5f), fvec3_mul_scalar(axis2,  0.5f))));

  vertices[0].texture_coords = fvec2(0.0f, 0.0f);
  vertices[1].texture_coords = fvec2(0.0f, 1.0f);
  vertices[2].texture_coords = fvec2(1.0f, 0.0f);
  vertices[3].texture_coords = fvec2(1.0f, 1.0f);

  vertices[0].texture_index = texture_index;
  vertices[1].texture_index = texture_index;
  vertices[2].texture_index = texture_index;
  vertices[3].texture_index = texture_index;

  vertices[0].light_level = light_level;
  vertices[1].light_level = light_level;
  vertices[2].light_level = light_level;
  vertices[3].light_level = light_level;

  chunk_mesh_builder_push_indices(chunk_mesh_builder, sizeof indices / sizeof indices[0], indices);
  chunk_mesh_builder_push_vertices(chunk_mesh_builder, sizeof vertices / sizeof vertices[0], vertices);
}

static inline struct tile *chunk_tile_lookup(struct chunk *chunk, ivec3_t position)
{
  if(position.z >= 0 && position.z < CHUNK_WIDTH)
    if(position.y >= 0 && position.y < CHUNK_WIDTH)
      if(position.x >= 0 && position.x < CHUNK_WIDTH)
        return &chunk->chunk_data->tiles[position.z][position.y][position.x];

  if(position.z == -1)          return chunk->bottom ? &chunk->bottom->chunk_data->tiles[CHUNK_WIDTH-1][position.y][position.x] : NULL;
  if(position.z == CHUNK_WIDTH) return chunk->top    ? &chunk->top   ->chunk_data->tiles[0]            [position.y][position.x] : NULL;
  if(position.y == -1)          return chunk->back   ? &chunk->back  ->chunk_data->tiles[position.z][CHUNK_WIDTH-1][position.x] : NULL;
  if(position.y == CHUNK_WIDTH) return chunk->front  ? &chunk->front ->chunk_data->tiles[position.z][0]            [position.x] : NULL;
  if(position.x == -1)          return chunk->left   ? &chunk->left  ->chunk_data->tiles[position.z][position.y][CHUNK_WIDTH-1] : NULL;
  if(position.x == CHUNK_WIDTH) return chunk->right  ? &chunk->right ->chunk_data->tiles[position.z][position.y][0]             : NULL;

  assert(0 && "Unreachable");
}

__attribute__((flatten))
void world_update_mesh(struct world *world, struct resource_pack *resource_pack)
{
  struct chunk_mesh_info
  {
    struct chunk              *chunk;
    struct chunk_mesh_builder  chunk_mesh_builder_opaque;
    struct chunk_mesh_builder  chunk_mesh_builder_transparent;
  };

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
          if(chunk_mesh_info_capacity == chunk_mesh_info_count)
          {
            chunk_mesh_info_capacity = chunk_mesh_info_capacity != 0 ? chunk_mesh_info_capacity * 2 : 1;
            chunk_mesh_infos         = realloc(chunk_mesh_infos, chunk_mesh_info_capacity * sizeof *chunk_mesh_infos);
          }

          struct chunk_mesh_info *chunk_mesh_info = &chunk_mesh_infos[chunk_mesh_info_count++];
          chunk_mesh_info->chunk = chunk;

          chunk_mesh_info->chunk_mesh_builder_opaque.vertices        = NULL;
          chunk_mesh_info->chunk_mesh_builder_opaque.vertex_count    = 0;
          chunk_mesh_info->chunk_mesh_builder_opaque.vertex_capacity = 0;
          chunk_mesh_info->chunk_mesh_builder_opaque.indices         = NULL;
          chunk_mesh_info->chunk_mesh_builder_opaque.index_count     = 0;
          chunk_mesh_info->chunk_mesh_builder_opaque.index_capacity  = 0;

          chunk_mesh_info->chunk_mesh_builder_transparent.vertices        = NULL;
          chunk_mesh_info->chunk_mesh_builder_transparent.vertex_count    = 0;
          chunk_mesh_info->chunk_mesh_builder_transparent.vertex_capacity = 0;
          chunk_mesh_info->chunk_mesh_builder_transparent.indices         = NULL;
          chunk_mesh_info->chunk_mesh_builder_transparent.index_count     = 0;
          chunk_mesh_info->chunk_mesh_builder_transparent.index_capacity  = 0;
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
        {
          ivec3_t normals[] = {
            ivec3(-1,  0,  0),
            ivec3( 1,  0,  0),
            ivec3( 0, -1,  0),
            ivec3( 0,  1,  0),
            ivec3( 0,  0, -1),
            ivec3( 0,  0,  1),
          };

          for(size_t j=0; j<6; ++j)
          {
            ivec3_t position  = ivec3(x, y, z);
            ivec3_t nposition = ivec3_add(position, normals[j]);

            const struct tile *tile  = chunk_tile_lookup(chunk_mesh_infos[i].chunk, position);
            const struct tile *ntile = chunk_tile_lookup(chunk_mesh_infos[i].chunk, nposition);

            fvec3_t  emit_position      = ivec3_as_fvec3(ivec3_add(ivec3_mul_scalar(chunk_mesh_infos[i].chunk->position, CHUNK_WIDTH), position));
            fvec3_t  emit_normal        = ivec3_as_fvec3(normals[j]);
            uint32_t emit_texture_index = 0;
            float    emit_light_level   = ntile ? (float)ntile->light_level / 15.0f : 1.0f;
            switch(j)
            {
              case 0: emit_texture_index = resource_pack->block_infos[tile->id].texture_left;   break;
              case 1: emit_texture_index = resource_pack->block_infos[tile->id].texture_right;  break;
              case 2: emit_texture_index = resource_pack->block_infos[tile->id].texture_back;   break;
              case 3: emit_texture_index = resource_pack->block_infos[tile->id].texture_front;  break;
              case 4: emit_texture_index = resource_pack->block_infos[tile->id].texture_bottom; break;
              case 5: emit_texture_index = resource_pack->block_infos[tile->id].texture_top;    break;
            }

            switch(resource_pack->block_infos[tile->id].type)
            {
              case BLOCK_TYPE_OPAQUE:
                if(!ntile || resource_pack->block_infos[ntile->id].type != BLOCK_TYPE_OPAQUE)
                  chunk_mesh_builder_emit_face(&chunk_mesh_infos[i].chunk_mesh_builder_opaque, emit_position, emit_normal, emit_texture_index, emit_light_level);
                break;
              case BLOCK_TYPE_TRANSPARENT:
                if(!ntile || (resource_pack->block_infos[ntile->id].type != BLOCK_TYPE_OPAQUE && resource_pack->block_infos[ntile->id].type != BLOCK_TYPE_TRANSPARENT))
                  chunk_mesh_builder_emit_face(&chunk_mesh_infos[i].chunk_mesh_builder_transparent, emit_position, emit_normal, emit_texture_index, emit_light_level);
                break;
              default:
                break;
            }

          }
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

      glGenVertexArrays(1, &chunk_mesh->vao_opaque);
      glGenBuffers(1, &chunk_mesh->vbo_opaque);
      glGenBuffers(1, &chunk_mesh->ibo_opaque);

      glGenVertexArrays(1, &chunk_mesh->vao_transparent);
      glGenBuffers(1, &chunk_mesh->vbo_transparent);
      glGenBuffers(1, &chunk_mesh->ibo_transparent);

      chunk_mesh_infos[i].chunk->chunk_mesh = chunk_mesh;
    }

    glBindVertexArray(chunk_mesh->vao_opaque);

    glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo_opaque);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertex_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertices, chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo_opaque);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_opaque.index_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_opaque.indices, chunk_mesh_infos[i].chunk_mesh_builder_opaque.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, light_level));

    chunk_mesh->count_opaque = chunk_mesh_infos[i].chunk_mesh_builder_opaque.index_count;

    glBindVertexArray(chunk_mesh->vao_transparent);

    glBindBuffer(GL_ARRAY_BUFFER, chunk_mesh->vbo_transparent);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertex_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertices, chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh->ibo_transparent);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_transparent.index_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_transparent.indices, chunk_mesh_infos[i].chunk_mesh_builder_transparent.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, light_level));

    chunk_mesh->count_transparent = chunk_mesh_infos[i].chunk_mesh_builder_transparent.index_count;
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
          glDeleteVertexArrays(1, &chunk->chunk_mesh->vao_opaque);
          glDeleteBuffers(1, &chunk->chunk_mesh->vbo_opaque);
          glDeleteBuffers(1, &chunk->chunk_mesh->ibo_opaque);

          glDeleteVertexArrays(1, &chunk->chunk_mesh->vao_transparent);
          glDeleteBuffers(1, &chunk->chunk_mesh->vbo_transparent);
          glDeleteBuffers(1, &chunk->chunk_mesh->ibo_transparent);

          free(chunk->chunk_mesh);

          chunk->chunk_mesh = NULL;
        }

  //////////////////
  /// 5: Cleanup ///
  //////////////////
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
  {
    free(chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertices);
    free(chunk_mesh_infos[i].chunk_mesh_builder_opaque.indices);

    free(chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertices);
    free(chunk_mesh_infos[i].chunk_mesh_builder_transparent.indices);
  }
  free(chunk_mesh_infos);
}
