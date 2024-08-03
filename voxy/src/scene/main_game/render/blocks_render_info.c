#include <voxy/scene/main_game/render/blocks_render_info.h>
#include <voxy/scene/main_game/render/assets.h>

#include <voxy/scene/main_game/types/registry.h>
#include <voxy/math/direction.h>
#include <voxy/graphics/gl.h>

#include <stdlib.h>

struct blocks_render_info *blocks_render_info_create(void)
{
  struct blocks_render_info *blocks_render_info = malloc(sizeof *blocks_render_info);
  blocks_mesh_init(&blocks_render_info->opaque_mesh);
  blocks_mesh_init(&blocks_render_info->transparent_mesh);
  return blocks_render_info;
}

void blocks_render_info_destroy(struct blocks_render_info *blocks_render_info)
{
  blocks_mesh_fini(&blocks_render_info->opaque_mesh);
  blocks_mesh_fini(&blocks_render_info->transparent_mesh);
  free(blocks_render_info);
}

void blocks_render_info_update(struct blocks_render_info *blocks_render_info, const struct chunk *chunk, const struct digger *digger)
{
  /// Cache block ids
  block_id_t block_ids[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
  for(int z = -1; z<CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        block_ids[z+1][y+1][x+1] = chunk_get_block_id_ex(chunk, ivec3(x, y, z));

  /// Cache block light_levels
  uint8_t block_light_levels[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
  for(int z = -1; z<CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        block_light_levels[z+1][y+1][x+1] = chunk_get_block_light_level_ex(chunk, ivec3(x, y, z));

  /// Cache block types
  block_id_t block_types[CHUNK_WIDTH+2][CHUNK_WIDTH+2][CHUNK_WIDTH+2];
  for(int z = -1; z<CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<CHUNK_WIDTH+1; ++x)
        block_types[z+1][y+1][x+1] = block_ids[z+1][y+1][x+1] != BLOCK_NONE ? query_block_info(block_ids[z+1][y+1][x+1])->type : BLOCK_TYPE_OPAQUE;

  /// Compute occlusion
  uint8_t occlusions[CHUNK_WIDTH+1][CHUNK_WIDTH+1][CHUNK_WIDTH+1];
  for(int z = 0; z<CHUNK_WIDTH+1; ++z)
    for(int y = 0; y<CHUNK_WIDTH+1; ++y)
      for(int x = 0; x<CHUNK_WIDTH+1; ++x)
      {
        occlusions[z][y][x] = 0;
        for(int dz = 0; dz<2; ++dz)
          for(int dy = 0; dy<2; ++dy)
            for(int dx = 0; dx<2; ++dx)
              if(block_types[z+dz][y+dy][x+dx] == BLOCK_TYPE_OPAQUE)
                occlusions[z][y][x] += 1;
      }

  // Meshing
  struct vertices opaque_vertices = {0};
  struct vertices transparent_vertices = {0};
  for(int z = 0; z<CHUNK_WIDTH; ++z)
    for(int y = 0; y<CHUNK_WIDTH; ++y)
      for(int x = 0; x<CHUNK_WIDTH; ++x)
      {
        const ivec3_t local_position = ivec3(x, y, z);
        const ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk->position, CHUNK_WIDTH), local_position);

        const block_id_t block_id = block_ids[z+1][y+1][x+1];
        const enum block_type block_type = block_types[z+1][y+1][x+1];

        const float damage = ivec3_eql(digger->position, global_position) ? digger->damage : 0.0f;

        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        {
          const ivec3_t normal = direction_as_ivec(direction);

          const uint8_t neighbour_block_light_level = block_light_levels[z+normal.z+1][y+normal.y+1][x+normal.x+1];
          const enum block_type neighbour_block_type = block_types[z+normal.z+1][y+normal.y+1][x+normal.x+1];

          if(block_type != BLOCK_TYPE_OPAQUE && block_type != BLOCK_TYPE_TRANSPARENT)
            continue;

          if(block_type == BLOCK_TYPE_OPAQUE && neighbour_block_type == BLOCK_TYPE_OPAQUE)
            continue;

          if(block_type == BLOCK_TYPE_TRANSPARENT && neighbour_block_type == BLOCK_TYPE_TRANSPARENT)
            continue;

          if(block_type == BLOCK_TYPE_TRANSPARENT && neighbour_block_type == BLOCK_TYPE_OPAQUE)
            continue;

          const ivec3_t center = global_position;

          const uint32_t normal_index = direction;
          const uint32_t texture_index = assets_get_block_texture_array_index(block_id, direction);

          const ivec3_t axis2 = normal.z != 0 ? ivec3(1, 0, 0) : ivec3(0, 0, 1);
          const ivec3_t axis1 = ivec3_cross(normal, axis2);

          const uint16_t light_level = neighbour_block_light_level;
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

          struct vertex vertex;
          vertex.center = center;
          vertex.normal_index_and_texture_index = normal_index | texture_index << 3;
          vertex.light_level_and_occlusion_counts = light_level;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[0][0] << 4;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[0][1] << 8;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[1][0] << 12;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[1][1] << 16;
          vertex.damage = damage;

          switch(block_type)
          {
          case BLOCK_TYPE_OPAQUE:      DYNAMIC_ARRAY_APPEND(opaque_vertices, vertex);      break;
          case BLOCK_TYPE_TRANSPARENT: DYNAMIC_ARRAY_APPEND(transparent_vertices, vertex); break;
          default: assert(0 && "Unreachable");
          }
        }
      }

  blocks_mesh_update(&blocks_render_info->opaque_mesh, opaque_vertices);
  blocks_mesh_update(&blocks_render_info->transparent_mesh, transparent_vertices);
}

void blocks_render_info_cull(struct blocks_render_info *blocks_render_info, const struct camera *camera)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fvec3_t min = fvec3(+INFINITY, +INFINITY, +INFINITY);
  fvec3_t max = fvec3(-INFINITY, -INFINITY, -INFINITY);
  for(int k=0; k<2; ++k)
    for(int j=0; j<2; ++j)
      for(int i=0; i<2; ++i)
      {
        fvec3_t point_world_space = fvec3_sub(ivec3_as_fvec3(ivec3_mul_scalar(ivec3_add(blocks_render_info->position, ivec3(i, j, k)), CHUNK_WIDTH)), fvec3(0.5f, 0.5f, 0.5f));
        fvec3_t point_clip_space  = fmat4_apply_fvec3_perspective_divide(VP, point_world_space);

        min = fvec3_min(min, point_clip_space);
        max = fvec3_max(max, point_clip_space);
      }

  blocks_render_info->culled = true;
  blocks_render_info->culled = blocks_render_info->culled && (min.x >= 1.0f || max.x <= -1.0f);
  blocks_render_info->culled = blocks_render_info->culled && (min.y >= 1.0f || max.y <= -1.0f);
  blocks_render_info->culled = blocks_render_info->culled && (min.z >= 1.0f || max.z <= -1.0f);
}

void blocks_render_info_render_begin(const struct camera *camera)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(camera), V);

  struct gl_program program = GL_PROGRAM_LOAD(chunk);
  glUseProgram(program.id);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, assets_get_block_texture_array().id);
}


void blocks_render_info_render_opaque(const struct blocks_render_info *blocks_render_info)
{
  blocks_mesh_render(&blocks_render_info->opaque_mesh);
}

void blocks_render_info_render_transparent(const struct blocks_render_info *blocks_render_info)
{
  blocks_mesh_render(&blocks_render_info->transparent_mesh);
}

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX blocks_render_info
#define SC_HASH_TABLE_NODE_TYPE struct blocks_render_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t blocks_render_info_key(struct blocks_render_info *blocks_render_info)
{
  return blocks_render_info->position;
}

size_t blocks_render_info_hash(ivec3_t position)
{
  return ivec3_hash(position);
}

int blocks_render_info_compare(ivec3_t position1, ivec3_t position2)
{
  return ivec3_compare(position1, position2);
}

void blocks_render_info_dispose(struct blocks_render_info *blocks_render_info)
{
  blocks_render_info_destroy(blocks_render_info);
}

