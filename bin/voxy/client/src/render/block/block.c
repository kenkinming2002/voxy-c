#include "block.h"
#include "camera/main.h"

#include <voxy/client/registry/block.h>

#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <string.h>

int block_renderer_init(struct block_renderer *block_renderer)
{
  const char **textures = NULL;

  GLenum program_targets[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
  const char *program_filepaths[] = {"bin/voxy/client/src/render/block/block.vert", "bin/voxy/client/src/render/block/block.frag"};
  if(gl_program_load(&block_renderer->program, 2, program_targets, program_filepaths) != 0)
    goto error0;

  const struct voxy_block_info *infos = voxy_query_block_all();
  block_renderer->texture_indices = malloc(arrlenu(infos) * sizeof *block_renderer->texture_indices);
  for(voxy_block_id_t id=0; id<arrlenu(infos); ++id)
    for(direction_t direction=0; direction<DIRECTION_COUNT; ++direction)
    {
      const char *texture = infos[id].textures[direction];
      if(texture)
      {
        uint32_t i;
        for(i=0; i<arrlenu(textures); ++i)
          if(strcmp(texture, textures[i]) == 0)
            break;

        if(i == arrlenu(textures))
          arrput(textures, texture);

        block_renderer->texture_indices[id][direction] = i;
      }
    }

  if(gl_array_texture_2d_load(&block_renderer->texture, arrlenu(textures), textures) != 0)
    goto error1;

  block_renderer->render_info_nodes = NULL;
  return 0;

error1:
  gl_program_fini(&block_renderer->program);
error0:
  arrfree(textures);
  return -1;
}

void block_renderer_fini(struct block_renderer *block_renderer)
{
  hmfree(block_renderer->render_info_nodes);
  gl_array_texture_2d_fini(&block_renderer->texture);
  gl_program_fini(&block_renderer->program);
}

static void discard_render_infos(struct block_renderer *block_renderer, ivec3_t center, int radius)
{
  profile_begin();

  size_t count = 0;

  struct block_render_info_node *new_render_info_nodes = NULL;
  for(ptrdiff_t i=0; i<hmlen(block_renderer->render_info_nodes); ++i)
  {
    ivec3_t position = block_renderer->render_info_nodes[i].key;
    struct block_render_info render_info = block_renderer->render_info_nodes[i].value;
    if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
      hmput(new_render_info_nodes, position, render_info);
    else
      block_render_info_destroy(render_info);
  }

  hmfree(block_renderer->render_info_nodes);
  block_renderer->render_info_nodes = new_render_info_nodes;

  if(count != 0)
    LOG_INFO("Render: Discard render infos for %zu block groups", count);

  profile_end("count", tformat("%zu", count));
}

static void update_render_infos(struct block_renderer *block_renderer, struct block_manager *block_manager, ivec3_t center, int radius)
{
  profile_begin();

  size_t count = 0;

  // Create render info for block groups inside render distance.
  for(int z = center.z - radius + 1; z <= center.z + radius - 1; ++z)
    for(int y = center.y - radius + 1; y <= center.y + radius - 1; ++y)
      for(int x = center.x - radius + 1; x <= center.x + radius - 1; ++x)
        if(count < 25)
        {
          const ivec3_t position = ivec3(x, y, z);
          if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
          {
            struct block_group_node *block_group_node = hmgetp_null(block_manager->block_group_nodes, position);
            if(!block_group_node)
              continue;

            struct block_group *block_group = block_group_node->value;
            bool remesh = block_group->remesh;

            struct block_render_info_node *render_info_node = hmgetp_null(block_renderer->render_info_nodes, position);
            if(!render_info_node)
            {
              hmput(block_renderer->render_info_nodes, position, block_render_info_create());
              render_info_node = hmgetp_null(block_renderer->render_info_nodes, position);
              remesh = true;
            }

            if(remesh)
            {
              struct block_render_info *render_info = &render_info_node->value;
              block_render_info_update(render_info, block_renderer, position, block_group);
              block_group->remesh = false;

              count += 1;
            }
          }
        }

  if(count != 0)
    LOG_INFO("Render: Updated render infos for %zu block groups", count);

  profile_end("count", tformat("%zu", count));
}

void block_renderer_update(struct block_renderer *block_renderer, struct block_manager *block_manager)
{
  const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(get_main_camera().transform.translation), VOXY_CHUNK_WIDTH);
  const int radius = 8;

  discard_render_infos(block_renderer, center, radius);
  update_render_infos(block_renderer, block_manager, center, radius);
}

void block_renderer_render(struct block_renderer *block_renderer)
{
  const struct camera camera = get_main_camera();

  const fmat4_t V = camera_view_matrix(&camera);
  const fmat4_t P = camera_projection_matrix(&camera);
  const fmat4_t VP = fmat4_mul(P, V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(block_renderer->program.id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, block_renderer->texture.id);

  for(ptrdiff_t i=0; i<hmlen(block_renderer->render_info_nodes); ++i)
    block_render_info_update_cull(block_renderer->render_info_nodes[i].key, &block_renderer->render_info_nodes[i].value, &camera);

  for(ptrdiff_t i=0; i<hmlen(block_renderer->render_info_nodes); ++i)
  {
    const fmat4_t M = fmat4_translate(ivec3_as_fvec3(ivec3_mul_scalar(block_renderer->render_info_nodes[i].key, VOXY_CHUNK_WIDTH)));
    const fmat4_t MV  = fmat4_mul(V, M);
    const fmat4_t MVP = fmat4_mul(VP, M);

    glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "MV"),  1, GL_TRUE, (const float *)&MV);
    glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

    block_mesh_render(&block_renderer->render_info_nodes[i].value.opaque_mesh);
  }

  for(ptrdiff_t i=0; i<hmlen(block_renderer->render_info_nodes); ++i)
  {
    const fmat4_t M = fmat4_translate(ivec3_as_fvec3(ivec3_mul_scalar(block_renderer->render_info_nodes[i].key, VOXY_CHUNK_WIDTH)));
    const fmat4_t MV  = fmat4_mul(V, M);
    const fmat4_t MVP = fmat4_mul(VP, M);

    glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "MV"),  1, GL_TRUE, (const float *)&MV);
    glUniformMatrix4fv(glGetUniformLocation(block_renderer->program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

    block_mesh_render(&block_renderer->render_info_nodes[i].value.transparent_mesh);
  }
}

