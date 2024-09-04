#include "render_info.h"
#include "block.h"

#include <libcommon/math/direction.h>
#include <libcommon/graphics/gl.h>

#include <stdlib.h>

struct block_render_info *block_render_info_create(void)
{
  struct block_render_info *block_render_info = malloc(sizeof *block_render_info);
  block_mesh_init(&block_render_info->opaque_mesh);
  block_mesh_init(&block_render_info->transparent_mesh);
  return block_render_info;
}

void block_render_info_destroy(struct block_render_info *block_render_info)
{
  block_mesh_fini(&block_render_info->opaque_mesh);
  block_mesh_fini(&block_render_info->transparent_mesh);
  free(block_render_info);
}

void block_render_info_update(struct block_render_info *block_render_info, struct voxy_block_registry *block_registry, struct block_renderer *block_renderer, const struct chunk *chunk)
{
  struct block_vertices opaque_vertices = {0};
  struct block_vertices transparent_vertices = {0};

  // Cache block ids
  uint8_t block_ids[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2];
  for(int z = -1; z<VOXY_CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<VOXY_CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<VOXY_CHUNK_WIDTH+1; ++x)
        block_ids[z+1][y+1][x+1] = chunk_get_block_id_ex(chunk, ivec3(x, y, z), 0);

  // Cache block light_levels
  uint8_t block_light_levels[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2];
  for(int z = -1; z<VOXY_CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<VOXY_CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<VOXY_CHUNK_WIDTH+1; ++x)
        block_light_levels[z+1][y+1][x+1] = chunk_get_block_light_level_ex(chunk, ivec3(x, y, z), 15);

  /// Compute occlusion
  uint8_t occlusions[VOXY_CHUNK_WIDTH+1][VOXY_CHUNK_WIDTH+1][VOXY_CHUNK_WIDTH+1];
  for(int z = 0; z<VOXY_CHUNK_WIDTH+1; ++z)
    for(int y = 0; y<VOXY_CHUNK_WIDTH+1; ++y)
      for(int x = 0; x<VOXY_CHUNK_WIDTH+1; ++x)
      {
        occlusions[z][y][x] = 0;
        for(int dz = 0; dz<2; ++dz)
          for(int dy = 0; dy<2; ++dy)
            for(int dx = 0; dx<2; ++dx)
              if(block_ids[z+dz][y+dy][x+dx] != 0)
                occlusions[z][y][x] += 1;
      }

  /// Meshing
  ///
  /// FIXME: Potential out of bound array access if we receive a block id from
  ///        the server that is not inside the registry.
  for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const ivec3_t local_position = ivec3(x, y, z);
        const ivec3_t global_position = ivec3_add(ivec3_mul_scalar(chunk->position, VOXY_CHUNK_WIDTH), local_position);

        const uint8_t block_id = block_ids[z+1][y+1][x+1];
        const uint8_t block_light_level = block_light_levels[z+1][y+1][x+1];
        const enum voxy_block_type block_type = block_registry->infos.items[block_id].type;

        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        {
          const ivec3_t normal = direction_as_ivec(direction);

          const uint8_t neighbour_block_id = block_ids[z+normal.z+1][y+normal.y+1][x+normal.x+1];
          const uint8_t neighbour_block_light_level = block_light_levels[z+normal.z+1][y+normal.y+1][x+normal.x+1];
          const enum voxy_block_type neighbour_block_type = block_registry->infos.items[neighbour_block_id].type;

          if(block_type == VOXY_BLOCK_TYPE_INVISIBLE || neighbour_block_type != VOXY_BLOCK_TYPE_INVISIBLE)
            continue;

          const ivec3_t center = global_position;

          const uint32_t normal_index = direction;
          const uint32_t texture_index = block_renderer->texture_indices[block_id][direction];

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

          struct block_vertex vertex;
          vertex.center = center;
          vertex.normal_index_and_texture_index = normal_index | texture_index << 3;
          vertex.light_level_and_occlusion_counts = light_level;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[0][0] << 4;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[0][1] << 8;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[1][0] << 12;
          vertex.light_level_and_occlusion_counts |= occlusion_counts[1][1] << 16;
          vertex.damage = 0.0f;

          DYNAMIC_ARRAY_APPEND(opaque_vertices, vertex);
        }
      }

  block_mesh_update(&block_render_info->opaque_mesh, opaque_vertices);
  block_mesh_update(&block_render_info->transparent_mesh, transparent_vertices);

  DYNAMIC_ARRAY_CLEAR(opaque_vertices);
  DYNAMIC_ARRAY_CLEAR(transparent_vertices);
}

void block_render_info_update_cull(struct block_render_info *block_render_info, const struct camera *camera)
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
        fvec3_t point_world_space = fvec3_sub(ivec3_as_fvec3(ivec3_mul_scalar(ivec3_add(block_render_info->position, ivec3(i, j, k)), VOXY_CHUNK_WIDTH)), fvec3(0.5f, 0.5f, 0.5f));
        fvec3_t point_clip_space  = fmat4_apply_fvec3_perspective_divide(VP, point_world_space);

        min = fvec3_min(min, point_clip_space);
        max = fvec3_max(max, point_clip_space);
      }

  block_render_info->culled = true;
  block_render_info->culled = block_render_info->culled && (min.x >= 1.0f || max.x <= -1.0f);
  block_render_info->culled = block_render_info->culled && (min.y >= 1.0f || max.y <= -1.0f);
  block_render_info->culled = block_render_info->culled && (min.z >= 1.0f || max.z <= -1.0f);
}

#define SC_HASH_TABLE_IMPLEMENTATION
#define SC_HASH_TABLE_PREFIX block_render_info
#define SC_HASH_TABLE_NODE_TYPE struct block_render_info
#define SC_HASH_TABLE_KEY_TYPE ivec3_t
#include <sc/hash_table.h>
#undef SC_HASH_TABLE_KEY_TYPE
#undef SC_HASH_TABLE_NODE_TYPE
#undef SC_HASH_TABLE_PREFIX
#undef SC_HASH_TABLE_IMPLEMENTATION

ivec3_t block_render_info_key(struct block_render_info *block_render_info) { return block_render_info->position; }
size_t block_render_info_hash(ivec3_t position) { return ivec3_hash(position); }
int block_render_info_compare(ivec3_t position1, ivec3_t position2) { return ivec3_compare(position1, position2); }
void block_render_info_dispose(struct block_render_info *block_render_info) { block_render_info_destroy(block_render_info); }

