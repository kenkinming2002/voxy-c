#include "block.h"

#include <libcommon/core/log.h>

#include <string.h>

int block_renderer_init(struct block_renderer *block_renderer, const struct voxy_block_registry *block_registry)
{
  DYNAMIC_ARRAY_DECLARE(textures, const char *);

  GLenum program_targets[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
  const char *program_filepaths[] = {"bin/voxy/client/src/render/block/block.vert", "bin/voxy/client/src/render/block/block.frag"};
  if(gl_program_load(&block_renderer->program, 2, program_targets, program_filepaths) != 0)
    goto error0;

  block_renderer->texture_indices = malloc(block_registry->infos.item_count * sizeof *block_renderer->texture_indices);
  for(block_id_t id=0; id<block_registry->infos.item_count; ++id)
    for(direction_t direction=0; direction<DIRECTION_COUNT; ++direction)
    {
      const char *texture = block_registry->infos.items[id].textures[direction];
      if(texture)
      {
        uint32_t i;
        for(i=0; i<textures.item_count; ++i)
          if(strcmp(texture, textures.items[i]) == 0)
            break;

        if(i == textures.item_count)
          DYNAMIC_ARRAY_APPEND(textures, texture);

        block_renderer->texture_indices[id][direction] = i;
      }
    }

  if(gl_array_texture_2d_load(&block_renderer->texture, textures.item_count, textures.items) != 0)
    goto error1;

  block_render_info_hash_table_init(&block_renderer->render_infos);
  return 0;

error1:
  gl_program_fini(&block_renderer->program);
error0:
  DYNAMIC_ARRAY_CLEAR(textures);
  return -1;
}

void block_renderer_fini(struct block_renderer *block_renderer)
{
  block_render_info_hash_table_dispose(&block_renderer->render_infos);
  gl_array_texture_2d_fini(&block_renderer->texture);
  gl_program_fini(&block_renderer->program);
}

void block_renderer_update(struct block_renderer *block_renderer, struct voxy_block_registry *block_registry, struct chunk_manager *chunk_manager, struct camera_manager *camera_manager)
{
  const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(camera_manager->camera.transform.translation), VOXY_CHUNK_WIDTH);
  const int radius = 8;

  unsigned discard_count = 0;
  unsigned update_count = 0;

  // Discard render info for chunks outside of render distance.
  for(size_t i=0; i<block_renderer->render_infos.bucket_count; ++i)
  {
    struct block_render_info **render_info = &block_renderer->render_infos.buckets[i].head;
    while(*render_info)
    {
      const ivec3_t position = (*render_info)->position;
      if(ivec3_length_squared(ivec3_sub(position, center)) > radius * radius)
      {
        struct block_render_info *old_render_info = *render_info;
        *render_info = (*render_info)->next;
        block_renderer->render_infos.load -= 1;
        block_render_info_destroy(old_render_info);

        discard_count += 1;
      }
      else
        render_info = &(*render_info)->next;
    }
  }

  // Create render info for chunks inside render distance.
  for(int z = center.z - radius + 1; z <= center.z + radius - 1; ++z)
    for(int y = center.y - radius + 1; y <= center.y + radius - 1; ++y)
      for(int x = center.x - radius + 1; x <= center.x + radius - 1; ++x)
      {
        const ivec3_t position = ivec3(x, y, z);
        if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
        {
          struct chunk *chunk = chunk_hash_table_lookup(&chunk_manager->chunks, position);
          if(!chunk)
            continue;

          struct block_render_info *render_info = block_render_info_hash_table_lookup(&block_renderer->render_infos, position);
          if(!render_info)
          {
            render_info = block_render_info_create();
            render_info->position = position;
            block_render_info_hash_table_insert_unchecked(&block_renderer->render_infos, render_info);
            block_render_info_update(render_info, block_registry, block_renderer, chunk);

            update_count += 1;
          }
        }
      }

  if(discard_count != 0)
    LOG_INFO("Render: Discard meshes for %u chunk", discard_count);

  if(update_count != 0)
    LOG_INFO("Render: Updated meshes for %u chunk", update_count);
}

void block_renderer_render(struct block_renderer *block_renderer, struct camera_manager *camera_manager)
{
  struct block_render_info *render_info;

  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&camera_manager->camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&camera_manager->camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(&camera_manager->camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(block_renderer->program.id);
  glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "V"),  1, GL_TRUE, (const float *)&V);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, block_renderer->texture.id);

  SC_HASH_TABLE_FOREACH(block_renderer->render_infos, render_info)
    block_render_info_update_cull(render_info, &camera_manager->camera);

  SC_HASH_TABLE_FOREACH(block_renderer->render_infos, render_info)
    block_mesh_render(&render_info->opaque_mesh);

  SC_HASH_TABLE_FOREACH(block_renderer->render_infos, render_info)
    block_mesh_render(&render_info->transparent_mesh);
}

