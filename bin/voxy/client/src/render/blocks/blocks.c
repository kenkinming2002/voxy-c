#include "blocks.h"

#include <libcommon/core/log.h>

#include <string.h>

int blocks_renderer_init(struct blocks_renderer *blocks_renderer, const struct block_registry *block_registry)
{
  DYNAMIC_ARRAY_DECLARE(textures, const char *);

  GLenum program_targets[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
  const char *program_filepaths[] = {"bin/voxy/client/src/render/blocks/blocks.vert", "bin/voxy/client/src/render/blocks/blocks.frag"};
  if(gl_program_load(&blocks_renderer->program, 2, program_targets, program_filepaths) != 0)
    goto error0;

  blocks_renderer->texture_indices = malloc(block_registry->infos.item_count * sizeof *blocks_renderer->texture_indices);
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

        blocks_renderer->texture_indices[id][direction] = i;
      }
    }

  if(gl_array_texture_2d_load(&blocks_renderer->texture, textures.item_count, textures.items) != 0)
    goto error1;

  blocks_render_info_hash_table_init(&blocks_renderer->render_infos);
  return 0;

error1:
  gl_program_fini(&blocks_renderer->program);
error0:
  DYNAMIC_ARRAY_CLEAR(textures);
  return -1;
}

void blocks_renderer_fini(struct blocks_renderer *blocks_renderer)
{
  blocks_render_info_hash_table_dispose(&blocks_renderer->render_infos);
  gl_array_texture_2d_fini(&blocks_renderer->texture);
  gl_program_fini(&blocks_renderer->program);
}

void blocks_renderer_update(struct blocks_renderer *blocks_renderer, struct block_registry *block_registry, struct chunk_manager *chunk_manager)
{
  const ivec3_t center = ivec3_zero();
  const int radius = 8;

  unsigned discard_count = 0;
  unsigned update_count = 0;

  // Discard render info for chunks outside of render distance.
  for(size_t i=0; i<blocks_renderer->render_infos.bucket_count; ++i)
  {
    struct blocks_render_info **render_info = &blocks_renderer->render_infos.buckets[i].head;
    while(*render_info)
    {
      const ivec3_t position = (*render_info)->position;
      if(ivec3_length_squared(ivec3_sub(position, center)) > radius * radius)
      {
        struct blocks_render_info *old_render_info = *render_info;
        *render_info = (*render_info)->next;
        blocks_renderer->render_infos.load -= 1;
        blocks_render_info_destroy(old_render_info);

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

          struct blocks_render_info *render_info = blocks_render_info_hash_table_lookup(&blocks_renderer->render_infos, position);
          if(!render_info)
          {
            render_info = blocks_render_info_create();
            render_info->position = position;
            blocks_render_info_hash_table_insert_unchecked(&blocks_renderer->render_infos, render_info);
            blocks_render_info_update(render_info, block_registry, blocks_renderer, chunk);

            update_count += 1;
          }
        }
      }

  if(discard_count != 0)
    LOG_INFO("Render: Discard meshes for %u chunk", discard_count);

  if(update_count != 0)
    LOG_INFO("Render: Updated meshes for %u chunk", update_count);
}

void blocks_renderer_render(struct blocks_renderer *blocks_renderer, const struct camera *camera)
{
  struct blocks_render_info *render_info;

  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(blocks_renderer->program.id);
  glUniformMatrix4fv(glGetUniformLocation(blocks_renderer->program.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(blocks_renderer->program.id, "V"),  1, GL_TRUE, (const float *)&V);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, blocks_renderer->texture.id);

  SC_HASH_TABLE_FOREACH(blocks_renderer->render_infos, render_info)
    blocks_render_info_update_cull(render_info, camera);

  SC_HASH_TABLE_FOREACH(blocks_renderer->render_infos, render_info)
    blocks_mesh_render(&render_info->opaque_mesh);

  SC_HASH_TABLE_FOREACH(blocks_renderer->render_infos, render_info)
    blocks_mesh_render(&render_info->transparent_mesh);
}

