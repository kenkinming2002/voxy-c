#include <voxy/main_game/chunk_remesh.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>

#include <voxy/types/chunk.h>
#include <voxy/types/chunk_data.h>
#include <voxy/types/chunk_mesh.h>
#include <voxy/types/player.h>

#include <voxy/math/vector.h>

#include <voxy/config.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

static inline int absi(int a)
{
  return a > 0 ? a : -a;
}

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

static inline void chunk_mesh_builder_push_vertex(struct chunk_mesh_builder *chunk_mesh_builder, struct chunk_mesh_vertex vertex)
{
  if(chunk_mesh_builder->vertex_capacity == chunk_mesh_builder->vertex_count)
  {
    chunk_mesh_builder->vertex_capacity = chunk_mesh_builder->vertex_capacity != 0 ? chunk_mesh_builder->vertex_capacity * 2 : 1;
    chunk_mesh_builder->vertices        = realloc(chunk_mesh_builder->vertices, chunk_mesh_builder->vertex_capacity * sizeof *chunk_mesh_builder->vertices);
  }
  chunk_mesh_builder->vertices[chunk_mesh_builder->vertex_count++] = vertex;
}

static inline void chunk_mesh_builder_push_index(struct chunk_mesh_builder *chunk_mesh_builder, uint32_t index)
{
  if(chunk_mesh_builder->index_capacity == chunk_mesh_builder->index_count)
  {
    chunk_mesh_builder->index_capacity = chunk_mesh_builder->index_capacity != 0 ? chunk_mesh_builder->index_capacity * 2 : 1;
    chunk_mesh_builder->indices        = realloc(chunk_mesh_builder->indices, chunk_mesh_builder->index_capacity * sizeof *chunk_mesh_builder->indices);
  }
  chunk_mesh_builder->indices[chunk_mesh_builder->index_count++] = index;
}

static inline struct block chunk_block_lookup(struct chunk *chunk, ivec3_t position)
{
  if(!chunk)
    return (struct block) { .id = BLOCK_NONE, .ether = false, .light_level = 0, };

  if(position.z >= 0 && position.z < CHUNK_WIDTH)
    if(position.y >= 0 && position.y < CHUNK_WIDTH)
      if(position.x >= 0 && position.x < CHUNK_WIDTH)
        return chunk->blocks[position.z][position.y][position.x];

  if(position.x == -1)          return chunk_block_lookup(chunk->left,   ivec3_add(position, ivec3(CHUNK_WIDTH, 0, 0)));
  if(position.x == CHUNK_WIDTH) return chunk_block_lookup(chunk->right,  ivec3_sub(position, ivec3(CHUNK_WIDTH, 0, 0)));
  if(position.y == -1)          return chunk_block_lookup(chunk->back,   ivec3_add(position, ivec3(0, CHUNK_WIDTH, 0)));
  if(position.y == CHUNK_WIDTH) return chunk_block_lookup(chunk->front,  ivec3_sub(position, ivec3(0, CHUNK_WIDTH, 0)));
  if(position.z == -1)          return chunk_block_lookup(chunk->bottom, ivec3_add(position, ivec3(0, 0, CHUNK_WIDTH)));
  if(position.z == CHUNK_WIDTH) return chunk_block_lookup(chunk->top,    ivec3_sub(position, ivec3(0, 0, CHUNK_WIDTH)));

  assert(0 && "Unreachable");
}

static inline size_t block_texture_index(const struct block_info *block_info, ivec3_t normal)
{
  if(normal.x == -1) return block_info->texture_left;
  if(normal.x ==  1) return block_info->texture_right;
  if(normal.y == -1) return block_info->texture_back;
  if(normal.y ==  1) return block_info->texture_front;
  if(normal.z == -1) return block_info->texture_bottom;
  if(normal.z ==  1) return block_info->texture_top;

  assert(0 && "Unreachable");
}

void update_chunk_remesh(void)
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

  ////////////////////////////////////////////////////
  /// 1: Collect all chunks that need to be meshed ///
  ////////////////////////////////////////////////////
  for(struct chunk *chunk = chunks_invalidated_mesh_head; chunk; chunk = chunk->mesh_next)
  {
    chunk->mesh_invalidated = false;
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
  chunks_invalidated_mesh_head = NULL;
  chunks_invalidated_mesh_tail = NULL;

  //////////////////////////////////////
  /// 2: Mesh all chunks in parallel ///
  //////////////////////////////////////
  #pragma omp parallel for
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
    for(int z = 0; z<CHUNK_WIDTH; ++z)
      for(int y = 0; y<CHUNK_WIDTH; ++y)
        for(int x = 0; x<CHUNK_WIDTH; ++x)
        {
          // Compiler plz ;)
          // Compiler plz ;)
          // Compiler plz ;)

          const struct block_info BLOCK_INFO_NONE = {
            .name = "invalid",
            .type = BLOCK_TYPE_OPAQUE,
          };

          struct block             blocks     [3][3][3];
          const struct block_info *block_infos[3][3][3];
          for(int dz = -1; dz<=1; ++dz)
            for(int dy = -1; dy<=1; ++dy)
              for(int dx = -1; dx<=1; ++dx)
              {
                blocks[1+dz][1+dy][1+dx] = chunk_block_lookup(chunk_mesh_infos[i].chunk, ivec3(x+dx, y+dy, z+dz));
                block_infos[1+dz][1+dy][1+dx] = blocks[1+dz][1+dy][1+dx].id  != BLOCK_NONE ? mod_block_info_get(blocks[1+dz][1+dy][1+dx].id)  : &BLOCK_INFO_NONE;
              }

          for(int dz = -1; dz<=1; ++dz)
            for(int dy = -1; dy<=1; ++dy)
              for(int dx = -1; dx<=1; ++dx)
                if(absi(dx) + absi(dy) + absi(dz) == 1)
                {
                  const struct block nblock = blocks[1+dz][1+dy][1+dx];

                  const struct block_info *block_info  = block_infos[1][1][1];
                  const struct block_info *nblock_info = block_infos[1+dz][1+dy][1+dx];

                  if(block_info->type == BLOCK_TYPE_OPAQUE      && nblock_info->type == BLOCK_TYPE_OPAQUE)      continue;
                  if(block_info->type == BLOCK_TYPE_TRANSPARENT && nblock_info->type == BLOCK_TYPE_OPAQUE)      continue;
                  if(block_info->type == BLOCK_TYPE_TRANSPARENT && nblock_info->type == BLOCK_TYPE_TRANSPARENT) continue;

                  struct chunk_mesh_builder *chunk_mesh_builder;
                  switch(block_info->type)
                  {
                  case BLOCK_TYPE_OPAQUE:      chunk_mesh_builder = &chunk_mesh_infos[i].chunk_mesh_builder_opaque;      break;
                  case BLOCK_TYPE_TRANSPARENT: chunk_mesh_builder = &chunk_mesh_infos[i].chunk_mesh_builder_transparent; break;
                  default:                     chunk_mesh_builder = NULL;                                                break;
                  }
                  if(!chunk_mesh_builder)
                    continue;

                  uint32_t index_base = chunk_mesh_builder->vertex_count;
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 0);
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 2);
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 1);
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 2);
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 3);
                  chunk_mesh_builder_push_index(chunk_mesh_builder, index_base + 1);

                  fvec3_t  center            = ivec3_as_fvec3(ivec3_add(ivec3_mul_scalar(chunk_mesh_infos[i].chunk->position, CHUNK_WIDTH), ivec3(x, y, z)));
                  ivec3_t  normal            = ivec3(dx, dy, dz);
                  uint32_t texture_index     = block_texture_index(block_info, ivec3(dx, dy, dz));
                  float    block_light_level = nblock.light_level / 15.0f;

                  ivec3_t axis2 = normal.z == 0 ? ivec3(0, 0, 1) : ivec3(1, 0, 0);
                  ivec3_t axis1 = ivec3_cross(normal, axis2);
                  for(int v = 0; v < 2; ++v)
                    for(int u = 0; u < 2; ++u)
                    {
                      ivec3_t dir1 = ivec3_mul_scalar(axis1, u * 2 - 1);
                      ivec3_t dir2 = ivec3_mul_scalar(axis2, v * 2 - 1);

                      struct chunk_mesh_vertex vertex;
                      vertex.position = fvec3_add(center, fvec3_mul_scalar(ivec3_as_fvec3(ivec3_add(normal, ivec3_add( dir1, dir2))), 0.5f));
                      vertex.texture_coords = fvec2(u, v);
                      vertex.texture_index = texture_index;

                      int occluded_count = 0;
                      for(int factor3 = 0; factor3 < 2; ++factor3)
                        for(int factor2 = 0; factor2 < 2; ++factor2)
                          for(int factor1 = 0; factor1 < 2; ++factor1)
                          {
                            ivec3_t offset = ivec3(1, 1, 1);
                            offset = ivec3_add(offset, ivec3_mul_scalar(dir1, factor1));
                            offset = ivec3_add(offset, ivec3_mul_scalar(dir2, factor2));
                            offset = ivec3_add(offset, ivec3_mul_scalar(normal, factor3));
                            if(block_infos[offset.z][offset.y][offset.x]->type == BLOCK_TYPE_OPAQUE)
                              occluded_count += 1;
                          }

                      float occlusion_factor = (float)occluded_count / (float)(2 * 2 * 2);
                      vertex.light_level = (block_light_level * 0.9f + 0.1f) * (1.0f - occlusion_factor);

                      chunk_mesh_builder_push_vertex(chunk_mesh_builder, vertex);
                    }
                }
        }

  //////////////////////////////
  /// 3: Upload chunk meshes ///
  //////////////////////////////
  for(size_t i=0; i<chunk_mesh_info_count; ++i)
  {
    struct chunk *chunk = chunk_mesh_infos[i].chunk;

    glBindVertexArray(chunk->vao_opaque);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_opaque);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertex_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertices, chunk_mesh_infos[i].chunk_mesh_builder_opaque.vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ibo_opaque);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_opaque.index_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_opaque.indices, chunk_mesh_infos[i].chunk_mesh_builder_opaque.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, light_level));

    chunk->count_opaque = chunk_mesh_infos[i].chunk_mesh_builder_opaque.index_count;

    glBindVertexArray(chunk->vao_transparent);

    glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_transparent);
    glBufferData(GL_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertex_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertices, chunk_mesh_infos[i].chunk_mesh_builder_transparent.vertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, chunk->ibo_transparent);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, chunk_mesh_infos[i].chunk_mesh_builder_transparent.index_count * sizeof *chunk_mesh_infos[i].chunk_mesh_builder_transparent.indices, chunk_mesh_infos[i].chunk_mesh_builder_transparent.indices, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glVertexAttribPointer (0, 3, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, position));
    glVertexAttribPointer (1, 2, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_coords));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, texture_index));
    glVertexAttribPointer (3, 1, GL_FLOAT, GL_FALSE, sizeof(struct chunk_mesh_vertex), (void *)offsetof(struct chunk_mesh_vertex, light_level));

    chunk->count_transparent = chunk_mesh_infos[i].chunk_mesh_builder_transparent.index_count;
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
