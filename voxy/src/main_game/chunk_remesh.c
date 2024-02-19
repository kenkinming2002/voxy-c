#include <voxy/main_game/chunk_remesh.h>

#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/chunk.h>
#include <voxy/main_game/config.h>

#include <voxy/math/vector.h>
#include <voxy/dynamic_array.h>

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

static inline int absi(int a)
{
  return a > 0 ? a : -a;
}

struct chunk_quad
{
  ivec3_t  center;
  uint32_t normal_index_and_texture_index;
  float    light_levels[2][2]; // FIXME: Occlusion
};

DYNAMIC_ARRAY_DEFINE(chunk_quads, struct chunk_quad);

struct chunk_mesh_info
{
  struct chunk *chunk;

  struct chunk_quads opaque_quads;
  struct chunk_quads transparent_quads;
};

DYNAMIC_ARRAY_DEFINE(chunk_mesh_infos, struct chunk_mesh_info);

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

static inline uint32_t get_normal_index(ivec3_t normal)
{
  if(normal.x == -1) return 0;
  if(normal.x ==  1) return 1;
  if(normal.y == -1) return 2;
  if(normal.y ==  1) return 3;
  if(normal.z == -1) return 4;
  if(normal.z ==  1) return 5;

  assert(0 && "Unreachable");
}

static inline uint32_t get_texture_index(ivec3_t normal, const struct block_info *block_info)
{
  if(normal.x == -1) return block_info->texture_left;
  if(normal.x ==  1) return block_info->texture_right;
  if(normal.y == -1) return block_info->texture_back;
  if(normal.y ==  1) return block_info->texture_front;
  if(normal.z == -1) return block_info->texture_bottom;
  if(normal.z ==  1) return block_info->texture_top;

  assert(0 && "Unreachable");
}

static void bind_quad_ebo()
{
  static GLuint ibo = 0;
  if(ibo == 0)
  {
    static uint8_t indices[] = { 0, 2, 1, 1, 2, 3 };

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
  }
  else
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
}

void update_chunk_remesh(void)
{
  //////////////////////////////////////////////////////
  ///// 1: Collect all chunks that need to be meshed ///
  //////////////////////////////////////////////////////
  struct chunk_mesh_infos chunk_mesh_infos = {0};
  for(struct chunk *chunk = chunks_invalidated_mesh_head; chunk; chunk = chunk->mesh_next)
  {
    DYNAMIC_ARRAY_APPEND(chunk_mesh_infos, ((struct chunk_mesh_info){
        .chunk = chunk,
        .opaque_quads = {0},
        .transparent_quads = {0},
    }));
    chunk->mesh_invalidated = false;
  }
  chunks_invalidated_mesh_head = NULL;
  chunks_invalidated_mesh_tail = NULL;

  ////////////////////////////////////////
  ///// 2: Mesh all chunks in parallel ///
  ////////////////////////////////////////
  #pragma omp parallel for
  for(size_t i=0; i<chunk_mesh_infos.item_count; ++i)
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
                blocks[1+dz][1+dy][1+dx] = chunk_block_lookup(chunk_mesh_infos.items[i].chunk, ivec3(x+dx, y+dy, z+dz));
                block_infos[1+dz][1+dy][1+dx] = blocks[1+dz][1+dy][1+dx].id  != BLOCK_NONE ? mod_block_info_get(blocks[1+dz][1+dy][1+dx].id)  : &BLOCK_INFO_NONE;
              }

          for(int dz = -1; dz<=1; ++dz)
            for(int dy = -1; dy<=1; ++dy)
              for(int dx = -1; dx<=1; ++dx)
                if(absi(dx) + absi(dy) + absi(dz) == 1)
                {
                  const ivec3_t normal = ivec3(dx, dy, dz);

                  const struct block nblock = blocks[1+dz][1+dy][1+dx];

                  const struct block_info *block_info  = block_infos[1][1][1];
                  const struct block_info *nblock_info = block_infos[1+dz][1+dy][1+dx];

                  if(block_info->type == BLOCK_TYPE_OPAQUE      && nblock_info->type == BLOCK_TYPE_OPAQUE)      continue;
                  if(block_info->type == BLOCK_TYPE_TRANSPARENT && nblock_info->type == BLOCK_TYPE_OPAQUE)      continue;
                  if(block_info->type == BLOCK_TYPE_TRANSPARENT && nblock_info->type == BLOCK_TYPE_TRANSPARENT) continue;

                  struct chunk_quads *chunk_quads;
                  switch(block_info->type)
                  {
                  case BLOCK_TYPE_OPAQUE:      chunk_quads = &chunk_mesh_infos.items[i].opaque_quads;      break;
                  case BLOCK_TYPE_TRANSPARENT: chunk_quads = &chunk_mesh_infos.items[i].transparent_quads; break;
                  default:
                    continue; // Nice
                  }

                  ivec3_t axis2 = normal.z != 0 ? ivec3(1, 0, 0) : ivec3(0, 0, 1);
                  ivec3_t axis1 = ivec3_cross(normal, axis2);

                  float block_light_level = nblock.light_level / 15.0f;
                  float light_levels[2][2];
                  for(int v = 0; v < 2; ++v)
                    for(int u = 0; u < 2; ++u)
                    {
                      ivec3_t dir1 = ivec3_mul_scalar(axis1, u * 2 - 1);
                      ivec3_t dir2 = ivec3_mul_scalar(axis2, v * 2 - 1);

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
                      light_levels[v][u] = (block_light_level * 0.9f + 0.1f) * (1.0f - occlusion_factor);
                    }

                  struct chunk_quad chunk_quad;
                  chunk_quad.center = ivec3_add(ivec3_mul_scalar(chunk_mesh_infos.items[i].chunk->position, CHUNK_WIDTH), ivec3(x, y, z));
                  chunk_quad.normal_index_and_texture_index = get_normal_index(normal) | get_texture_index(normal, block_info) << 3;
                  for(int v = 0; v < 2; ++v)
                    for(int u = 0; u < 2; ++u)
                      chunk_quad.light_levels[v][u] = light_levels[v][u];

                  DYNAMIC_ARRAY_APPEND(*chunk_quads, chunk_quad);

                }
        }

  ////////////////////////////////
  ///// 3: Upload chunk meshes ///
  ////////////////////////////////
  for(size_t i=0; i<chunk_mesh_infos.item_count; ++i)
  {
    struct chunk       *chunk                   = chunk_mesh_infos.items[i].chunk;
    struct chunk_quads *chunk_opaque_quads      = &chunk_mesh_infos.items[i].opaque_quads;
    struct chunk_quads *chunk_transparent_quads = &chunk_mesh_infos.items[i].transparent_quads;

    // Opaque
    {
      glBindVertexArray(chunk->vao_opaque);
      glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_opaque);
      glBufferData(GL_ARRAY_BUFFER, chunk_opaque_quads->item_count * sizeof *chunk_opaque_quads->items, chunk_opaque_quads->items, GL_DYNAMIC_DRAW);

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);

      glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT,    sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, center));
      glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, normal_index_and_texture_index));
      glVertexAttribPointer (2, 4, GL_FLOAT, GL_FALSE, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, light_levels));

      glVertexAttribDivisor(0, 1);
      glVertexAttribDivisor(1, 1);
      glVertexAttribDivisor(2, 1);

      bind_quad_ebo();

      chunk->count_opaque = chunk_opaque_quads->item_count;
    }

    // Transparent
    {
      glBindVertexArray(chunk->vao_transparent);
      glBindBuffer(GL_ARRAY_BUFFER, chunk->vbo_transparent);
      glBufferData(GL_ARRAY_BUFFER, chunk_transparent_quads->item_count * sizeof *chunk_transparent_quads->items, chunk_transparent_quads->items, GL_DYNAMIC_DRAW);

      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glEnableVertexAttribArray(2);

      glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT,    sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, center));
      glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT,    sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, normal_index_and_texture_index));
      glVertexAttribPointer (2, 4, GL_FLOAT, GL_FALSE, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, light_levels));

      glVertexAttribDivisor(0, 1);
      glVertexAttribDivisor(1, 1);
      glVertexAttribDivisor(2, 1);

      bind_quad_ebo();

      chunk->count_transparent = chunk_transparent_quads->item_count;
    }
  }

  ////////////////////
  ///// 5: Cleanup ///
  ////////////////////
  for(size_t i=0; i<chunk_mesh_infos.item_count; ++i)
  {
    free(chunk_mesh_infos.items[i].transparent_quads.items);
    free(chunk_mesh_infos.items[i].opaque_quads.items);
  }
  free(chunk_mesh_infos.items);
}
