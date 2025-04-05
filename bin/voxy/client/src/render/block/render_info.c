#include "render_info.h"
#include "block.h"

#include <libmath/direction.h>
#include <libgfx/gl.h>

#include <stb_ds.h>

struct block_render_info block_render_info_create(void)
{
  struct block_render_info block_render_info = {0};
  block_mesh_init(&block_render_info.opaque_mesh);
  block_mesh_init(&block_render_info.transparent_mesh);
  return block_render_info;
}

void block_render_info_destroy(struct block_render_info block_render_info)
{
  block_mesh_fini(&block_render_info.opaque_mesh);
  block_mesh_fini(&block_render_info.transparent_mesh);
}

static void prefetch_block_ids(const struct block_group *block_group, uint8_t ids[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2])
{
  for(int z = -1; z<VOXY_CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<VOXY_CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<VOXY_CHUNK_WIDTH+1; ++x)
        ids[z+1][y+1][x+1] = block_group_get_block_id_ex(block_group, ivec3(x, y, z), 0);
}

static void prefetch_block_light_levels(const struct block_group *block_group, uint8_t ids[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2])
{
  for(int z = -1; z<VOXY_CHUNK_WIDTH+1; ++z)
    for(int y = -1; y<VOXY_CHUNK_WIDTH+1; ++y)
      for(int x = -1; x<VOXY_CHUNK_WIDTH+1; ++x)
        ids[z+1][y+1][x+1] = block_group_get_block_light_level_ex(block_group, ivec3(x, y, z), 15);
}

static uint8_t get_prefetched(uint8_t infos[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2], ivec3_t position)
{
  return infos[position.z+1][position.y+1][position.x+1];
}

void block_render_info_update(struct block_render_info *block_render_info, struct voxy_block_registry *block_registry, struct block_renderer *block_renderer, ivec3_t position, const struct block_group *block_group)
{
  struct block_vertex *opaque_vertices = NULL;
  struct block_vertex *transparent_vertices = NULL;

  // Cache block ids
  uint8_t ids[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2];
  prefetch_block_ids(block_group, ids);

  // Cahce block light levels
  uint8_t light_levels[VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2][VOXY_CHUNK_WIDTH+2];
  prefetch_block_light_levels(block_group, light_levels);

  /// Meshing
  for(int z = 0; z<VOXY_CHUNK_WIDTH; ++z)
    for(int y = 0; y<VOXY_CHUNK_WIDTH; ++y)
      for(int x = 0; x<VOXY_CHUNK_WIDTH; ++x)
      {
        const ivec3_t local_position = ivec3(x, y, z);
        const ivec3_t global_position = ivec3_add(ivec3_mul_scalar(position, VOXY_CHUNK_WIDTH), local_position);

        const uint8_t block_id = get_prefetched(ids, local_position);
        const enum voxy_block_type block_type = voxy_block_registry_query_block(block_registry, block_id).type;

        #pragma omp unroll
        for(direction_t direction = 0; direction < DIRECTION_COUNT; ++direction)
        {
          const ivec3_t normal = direction_as_ivec(direction);

          const uint8_t neighbour_block_id = get_prefetched(ids, ivec3_add(local_position, normal));
          const enum voxy_block_type neighbour_block_type = voxy_block_registry_query_block(block_registry, neighbour_block_id).type;

          if(block_type == VOXY_BLOCK_TYPE_INVISIBLE || neighbour_block_type != VOXY_BLOCK_TYPE_INVISIBLE)
            continue;

          const ivec3_t center = global_position;

          const uint32_t normal_index = direction;
          const uint32_t texture_index = block_renderer->texture_indices[block_id][direction];

          const ivec3_t outer_axis3 = normal;                                               // z-axis
          const ivec3_t outer_axis2 = outer_axis3.z != 0 ? ivec3(1, 0, 0) : ivec3(0, 0, 1); // y-axis
          const ivec3_t outer_axis1 = ivec3_cross(outer_axis3, outer_axis2);                // x-axis

          uint8_t outer_local_light_levels[2][2] = {0};
          uint8_t outer_local_counts[2][2] = {0};

          #pragma omp unroll
          for(int v = 0; v < 2; ++v)
            #pragma omp unroll
            for(int u = 0; u < 2; ++u)
            {
              const ivec3_t inner_axis3 = outer_axis3;                              // z-axis
              const ivec3_t inner_axis2 = ivec3_mul_scalar(outer_axis2, v * 2 - 1); // y-axis
              const ivec3_t inner_axis1 = ivec3_mul_scalar(outer_axis1, u * 2 - 1); // x-axis

              uint8_t inner_local_ids[2][2][2];
              uint8_t inner_local_light_levels[2][2][2];

              #pragma omp unroll
              for(int dz = 0; dz<2; ++dz)
                #pragma omp unroll
                for(int dy = 0; dy<2; ++dy)
                  #pragma omp unroll
                  for(int dx = 0; dx<2; ++dx)
                  {
                    ivec3_t position = local_position;

                    position = ivec3_add(position, ivec3_mul_scalar(inner_axis3, dz));
                    position = ivec3_add(position, ivec3_mul_scalar(inner_axis2, dy));
                    position = ivec3_add(position, ivec3_mul_scalar(inner_axis1, dx));

                    inner_local_ids[dz][dy][dx] = get_prefetched(ids, position);
                    inner_local_light_levels[dz][dy][dx] = get_prefetched(light_levels, position);

                  }


              bool final = false;

              outer_local_light_levels[v][u] += inner_local_light_levels[1][0][0];
              outer_local_counts[v][u] += 1;

              if(inner_local_ids[1][0][1] == 0)
              {
                outer_local_light_levels[v][u] += inner_local_light_levels[1][0][1];
                outer_local_counts[v][u] += 1;
                if(inner_local_ids[0][0][1] == 0)
                {
                  outer_local_light_levels[v][u] += inner_local_light_levels[0][0][1];
                  outer_local_counts[v][u] += 1;
                  final = true;
                }
              }

              if(inner_local_ids[1][1][0] == 0)
              {
                outer_local_light_levels[v][u] += inner_local_light_levels[1][1][0];
                outer_local_counts[v][u] += 1;
                if(inner_local_ids[0][1][0] == 0)
                {
                  outer_local_light_levels[v][u] += inner_local_light_levels[0][1][0];
                  outer_local_counts[v][u] += 1;
                  final = true;
                }
              }

              if((inner_local_ids[1][0][1] == 0 || inner_local_ids[1][1][0] == 0) && inner_local_ids[1][1][1] == 0)
              {
                outer_local_light_levels[v][u] += inner_local_light_levels[1][1][1];
                outer_local_counts[v][u] += 1;
                final = true;
              }

              if(final && inner_local_ids[0][1][1] == 0)
              {
                outer_local_light_levels[v][u] += inner_local_light_levels[0][1][1];
                outer_local_counts[v][u] += 1;
              }
            }

          struct block_vertex vertex;

          vertex.center = center;

          vertex.metadata1 = 0;

          vertex.metadata1 |= (uint32_t)outer_local_light_levels[0][0] << (0 * 8);
          vertex.metadata1 |= (uint32_t)outer_local_light_levels[0][1] << (1 * 8);
          vertex.metadata1 |= (uint32_t)outer_local_light_levels[1][0] << (2 * 8);;
          vertex.metadata1 |= (uint32_t)outer_local_light_levels[1][1] << (3 * 8);;

          vertex.metadata2 = 0;

          vertex.metadata2 |= (uint32_t)outer_local_counts[0][0] << (0 * 4);
          vertex.metadata2 |= (uint32_t)outer_local_counts[0][1] << (1 * 4);
          vertex.metadata2 |= (uint32_t)outer_local_counts[1][0] << (2 * 4);
          vertex.metadata2 |= (uint32_t)outer_local_counts[1][1] << (3 * 4);

          vertex.metadata2 |= normal_index << 16;
          vertex.metadata2 |= texture_index << 19;

          vertex.damage = 0.0f;

          arrput(opaque_vertices, vertex);
        }
      }

  block_mesh_update(&block_render_info->opaque_mesh, opaque_vertices);
  block_mesh_update(&block_render_info->transparent_mesh, transparent_vertices);

  arrfree(opaque_vertices);
  arrfree(transparent_vertices);
}

void block_render_info_update_cull(ivec3_t position, struct block_render_info *block_render_info, const struct camera *camera)
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
        fvec3_t point_world_space = fvec3_sub(ivec3_as_fvec3(ivec3_mul_scalar(ivec3_add(position, ivec3(i, j, k)), VOXY_CHUNK_WIDTH)), fvec3(0.5f, 0.5f, 0.5f));
        fvec3_t point_clip_space  = fmat4_apply_fvec3_perspective_divide(VP, point_world_space);

        min = fvec3_min(min, point_clip_space);
        max = fvec3_max(max, point_clip_space);
      }

  block_render_info->culled = true;
  block_render_info->culled = block_render_info->culled && (min.x >= 1.0f || max.x <= -1.0f);
  block_render_info->culled = block_render_info->culled && (min.y >= 1.0f || max.y <= -1.0f);
  block_render_info->culled = block_render_info->culled && (min.z >= 1.0f || max.z <= -1.0f);
}

