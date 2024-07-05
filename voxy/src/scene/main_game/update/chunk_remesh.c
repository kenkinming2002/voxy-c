#include <voxy/scene/main_game/update/chunk_remesh.h>

#include <voxy/scene/main_game/config.h>
#include <voxy/scene/main_game/mod.h>
#include <voxy/scene/main_game/render/assets.h>

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/invalidate.h>

#include <voxy/core/log.h>
#include <voxy/math/vector.h>
#include <voxy/dynamic_array.h>

#include <time.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

struct chunk_quad
{
  ivec3_t center;

  // Bits 0..2: normal index
  // Bits 3.15: texture index
  uint32_t normal_index_and_texture_index;

  // Bits 0..3:   base light level
  // Bits 4..7:   occlusion count 0
  // Bits 8..11:  occlusion count 1
  // Bits 12..15: occlusion count 2
  // Bits 16..20: occlusion count 3
  uint32_t light_level_and_occlusion_counts;
};
DYNAMIC_ARRAY_DEFINE(chunk_quads, struct chunk_quad);

static inline ivec3_t face_get_normal(enum block_face face)
{
  switch(face)
  {
  case BLOCK_FACE_LEFT:   return ivec3(-1,  0,  0);
  case BLOCK_FACE_RIGHT:  return ivec3( 1,  0,  0);
  case BLOCK_FACE_BACK:   return ivec3( 0, -1,  0);
  case BLOCK_FACE_FRONT:  return ivec3( 0,  1,  0);
  case BLOCK_FACE_BOTTOM: return ivec3( 0,  0, -1);
  case BLOCK_FACE_TOP:    return ivec3( 0,  0,  1);
  default:
    assert(0 && "Unreachable");
  }
}

static void chunk_build_mesh(struct chunk *chunk, struct chunk_quads *opaque_quads, struct chunk_quads *transparent_quads)
{
  struct block blocks[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
  for(int z = -1; z<CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<CHUNK_WIDTH+1; ++x)
      {
        struct block *block = chunk_get_block(chunk, ivec3(x, y, z));
        if(!block)
        {
          blocks[z+1][y+1][x+1].id          = BLOCK_NONE;
          blocks[z+1][y+1][x+1].ether       = false;
          blocks[z+1][y+1][x+1].light_level = 0;
        }
        else
          blocks[z+1][y+1][x+1] = *block;
      }

  enum block_type block_types[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
  for(int z = -1; z<CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        block_types[z+1][y+1][x+1] = blocks[z+1][y+1][x+1].id != BLOCK_NONE ? query_block_info(blocks[z+1][y+1][x+1].id)->type : BLOCK_TYPE_OPAQUE;

  uint8_t occlusions[CHUNK_WIDTH+1][CHUNK_WIDTH+1][CHUNK_WIDTH+1];
  for(int z = 0; z<CHUNK_WIDTH+1; ++z)
    for(int y = 0; y<CHUNK_WIDTH+1; ++y)
      for(int x = 0; x<CHUNK_WIDTH+1; ++x)
      {
        occlusions[z][y][x] = 0;
        for(int dz = 0; dz<2; ++dz)
          for(int dy = 0; dy<2; ++dy)
            for(int dx = 0; dx<2; ++dx)
              occlusions[z][y][x] += block_types[z+dz][y+dy][x+dx] == BLOCK_TYPE_OPAQUE;
      }

  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
        #pragma omp unroll
        for(enum block_face face = 0; face<BLOCK_FACE_COUNT; ++face)
        {
          const ivec3_t normal = face_get_normal(face);

          const struct block      block      = blocks     [z+1][y+1][x+1];
          const enum   block_type block_type = block_types[z+1][y+1][x+1];

          const struct block      nblock      = blocks     [z+normal.z+1][y+normal.y+1][x+normal.x+1];
          const enum   block_type nblock_type = block_types[z+normal.z+1][y+normal.y+1][x+normal.x+1];

          if(block_type != BLOCK_TYPE_OPAQUE && block_type != BLOCK_TYPE_TRANSPARENT) continue;

          if(block_type == BLOCK_TYPE_OPAQUE      && nblock_type == BLOCK_TYPE_OPAQUE)      continue;
          if(block_type == BLOCK_TYPE_TRANSPARENT && nblock_type == BLOCK_TYPE_OPAQUE)      continue;
          if(block_type == BLOCK_TYPE_TRANSPARENT && nblock_type == BLOCK_TYPE_TRANSPARENT) continue;

          struct chunk_quad chunk_quad;
          chunk_quad.center = ivec3_add(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH), ivec3(x, y, z));

          const uint32_t normal_index  = face;
          const uint32_t texture_index = assets_get_block_texture_array_index(block.id, face);
          chunk_quad.normal_index_and_texture_index = normal_index | texture_index << 3;

          const ivec3_t axis2 = normal.z != 0 ? ivec3(1, 0, 0) : ivec3(0, 0, 1);
          const ivec3_t axis1 = ivec3_cross(normal, axis2);

          uint16_t light_level = nblock.light_level;
          uint16_t occlusion_counts[2][2];

          for(int v = 0; v < 2; ++v)
            for(int u = 0; u < 2; ++u)
            {
              // Fixed-point!?
              ivec3_t position;

              position = ivec3_mul_scalar(ivec3(x, y, z), 2);
              position = ivec3_add(position, normal);
              position = ivec3_add(position, ivec3_mul_scalar(axis1, u * 2 - 1));
              position = ivec3_add(position, ivec3_mul_scalar(axis2, v * 2 - 1));
              position = ivec3_add(position, ivec3(1, 1, 1));
              position = ivec3_div_scalar(position, 2);

              occlusion_counts[v][u] = occlusions[position.z][position.y][position.x];
            }

          chunk_quad.light_level_and_occlusion_counts = light_level | occlusion_counts[0][0] << 4
                                                                    | occlusion_counts[0][1] << 8
                                                                    | occlusion_counts[1][0] << 12
                                                                    | occlusion_counts[1][1] << 16;

          switch(block_type)
          {
          case BLOCK_TYPE_OPAQUE:      DYNAMIC_ARRAY_APPEND(*opaque_quads, chunk_quad);      break;
          case BLOCK_TYPE_TRANSPARENT: DYNAMIC_ARRAY_APPEND(*transparent_quads, chunk_quad); break;
          default:
            assert(0 && "Unreachable");
          }
        }
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

static void chunk_upload_mesh(struct chunk *chunk, struct chunk_quads *opaque_quads, struct chunk_quads *transparent_quads)
{
  if(!chunk->render_info)
  {
    chunk->render_info = malloc(sizeof *chunk->render_info);

    glGenVertexArrays(1, &chunk->render_info->opaque_mesh.vao);
    glGenBuffers(1, &chunk->render_info->opaque_mesh.vbo);
    chunk->render_info->opaque_mesh.count = 0;

    glGenVertexArrays(1, &chunk->render_info->transparent_mesh.vao);
    glGenBuffers(1, &chunk->render_info->transparent_mesh.vbo);
    chunk->render_info->transparent_mesh.count = 0;
  }

  // Opaque
  {
    glBindVertexArray(chunk->render_info->opaque_mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->render_info->opaque_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, opaque_quads->item_count * sizeof *opaque_quads->items, opaque_quads->items, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, center));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, normal_index_and_texture_index));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, light_level_and_occlusion_counts));

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    bind_quad_ebo();

    chunk->render_info->opaque_mesh.count = opaque_quads->item_count;
  }

  // Transparent
  {
    glBindVertexArray(chunk->render_info->transparent_mesh.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk->render_info->transparent_mesh.vbo);
    glBufferData(GL_ARRAY_BUFFER, transparent_quads->item_count * sizeof *transparent_quads->items, transparent_quads->items, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribIPointer(0, 3, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, center));
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, normal_index_and_texture_index));
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT, sizeof(struct chunk_quad), (void *)offsetof(struct chunk_quad, light_level_and_occlusion_counts));

    glVertexAttribDivisor(0, 1);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    bind_quad_ebo();

    chunk->render_info->transparent_mesh.count = transparent_quads->item_count;
  }
}

void update_chunk_remesh(void)
{
  struct job
  {
    struct chunk *chunk;
    struct chunk_quads opaque_quads;
    struct chunk_quads transparent_quads;
  };
  DYNAMIC_ARRAY_DECLARE(jobs, struct job);

  for(struct chunk *chunk = chunks_invalidated_mesh_head; chunk; chunk = chunk->mesh_next)
  {
    if(chunk->data)
      DYNAMIC_ARRAY_APPEND(jobs, ((struct job){
          .chunk = chunk,
          .opaque_quads      = {0},
          .transparent_quads = {0},
      }));
    chunk->mesh_invalidated = false;
  }
  chunks_invalidated_mesh_head = NULL;
  chunks_invalidated_mesh_tail = NULL;

  /*
   * Remeshing of chunk is splitted into two step which is represented by the
   * two functions chunk_build_mesh() and chunk_upload_mesh(). The difference
   * being that chunk_build_mesh() is thread-safe while chunk_upload_mesh() is
   * not. More precisely, chunk_upload_mesh() is responsible for updating mesh
   * data to GPU using OpenGL which is not thread-safe. Unfortunate.
   */

  size_t count = jobs.item_count;
  clock_t begin = clock();

  for(size_t i=0; i<jobs.item_count; ++i) chunk_build_mesh (jobs.items[i].chunk, &jobs.items[i].opaque_quads, &jobs.items[i].transparent_quads);
  for(size_t i=0; i<jobs.item_count; ++i) chunk_upload_mesh(jobs.items[i].chunk, &jobs.items[i].opaque_quads, &jobs.items[i].transparent_quads);
  for(size_t i=0; i<jobs.item_count; ++i)
  {
    free(jobs.items[i].transparent_quads.items);
    free(jobs.items[i].opaque_quads.items);
  }
  free(jobs.items);

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Chunk Mesher: Processed %zu chunks in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);
}
