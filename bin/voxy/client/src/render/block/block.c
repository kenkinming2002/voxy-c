#include "block.h"
#include "render_info.h"

#include "camera/main.h"
#include "chunk/block/manager.h"

#include <voxy/client/registry/block.h>

#include <libcore/log.h>
#include <libcore/profile.h>
#include <libcore/format.h>

#include <stb_ds.h>

#include <string.h>

struct block_render_info_node
{
  ivec3_t key;
  struct block_render_info value;
};


static struct gl_program program;

static struct gl_array_texture_2d texture;
static uint32_t (*texture_indices)[DIRECTION_COUNT];

static struct block_render_info_node *render_info_nodes;

void block_renderer_init(void)
{
  const char **textures = NULL;

  GLenum program_targets[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
  const char *program_filepaths[] = {"bin/voxy/client/src/render/block/block.vert", "bin/voxy/client/src/render/block/block.frag"};
  if(gl_program_load(&program, 2, program_targets, program_filepaths) != 0)
    exit(EXIT_FAILURE);

  const struct voxy_block_info *infos = voxy_query_block_all();
  texture_indices = malloc(arrlenu(infos) * sizeof *texture_indices);
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

        texture_indices[id][direction] = i;
      }
    }

  if(gl_array_texture_2d_load(&texture, arrlenu(textures), textures) != 0)
    exit(EXIT_FAILURE);
}

static void discard_render_infos(ivec3_t center, int radius)
{
  profile_begin();

  size_t count = 0;

  struct block_render_info_node *new_render_info_nodes = NULL;
  for(ptrdiff_t i=0; i<hmlen(render_info_nodes); ++i)
  {
    ivec3_t position = render_info_nodes[i].key;
    struct block_render_info render_info = render_info_nodes[i].value;
    if(ivec3_length_squared(ivec3_sub(position, center)) <= radius * radius)
      hmput(new_render_info_nodes, position, render_info);
    else
      block_render_info_destroy(render_info);
  }

  hmfree(render_info_nodes);
  render_info_nodes = new_render_info_nodes;

  if(count != 0)
    LOG_INFO("Render: Discard render infos for %zu block groups", count);

  profile_end("count", tformat("%zu", count));
}

static void update_render_infos(ivec3_t center, int radius)
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
            struct block_group *block_group = get_block_group(position);
            if(!block_group)
              continue;;

            bool remesh = block_group->remesh;

            struct block_render_info_node *render_info_node = hmgetp_null(render_info_nodes, position);
            if(!render_info_node)
            {
              hmput(render_info_nodes, position, block_render_info_create());
              render_info_node = hmgetp_null(render_info_nodes, position);
              remesh = true;
            }

            if(remesh)
            {
              struct block_render_info *render_info = &render_info_node->value;
              block_render_info_update(render_info, position, block_group);
              block_group->remesh = false;

              count += 1;
            }
          }
        }

  if(count != 0)
    LOG_INFO("Render: Updated render infos for %zu block groups", count);

  profile_end("count", tformat("%zu", count));
}

void block_renderer_update(void)
{
  const ivec3_t center = ivec3_div_scalar(fvec3_as_ivec3_round(get_main_camera().transform.translation), VOXY_CHUNK_WIDTH);
  const int radius = 8;

  discard_render_infos(center, radius);
  update_render_infos(center, radius);
}

void render_block(void)
{
  const struct camera camera = get_main_camera();

  const fmat4_t V = camera_view_matrix(&camera);
  const fmat4_t P = camera_projection_matrix(&camera);
  const fmat4_t VP = fmat4_mul(P, V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(program.id);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, texture.id);

  for(ptrdiff_t i=0; i<hmlen(render_info_nodes); ++i)
    block_render_info_update_cull(render_info_nodes[i].key, &render_info_nodes[i].value, &camera);

  for(ptrdiff_t i=0; i<hmlen(render_info_nodes); ++i)
  {
    const fmat4_t M = fmat4_translate(ivec3_as_fvec3(ivec3_mul_scalar(render_info_nodes[i].key, VOXY_CHUNK_WIDTH)));
    const fmat4_t MV  = fmat4_mul(V, M);
    const fmat4_t MVP = fmat4_mul(VP, M);

    glUniformMatrix4fv(glGetUniformLocation(program.id, "MV"),  1, GL_TRUE, (const float *)&MV);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

    block_mesh_render(&render_info_nodes[i].value.opaque_mesh);
  }

  for(ptrdiff_t i=0; i<hmlen(render_info_nodes); ++i)
  {
    const fmat4_t M = fmat4_translate(ivec3_as_fvec3(ivec3_mul_scalar(render_info_nodes[i].key, VOXY_CHUNK_WIDTH)));
    const fmat4_t MV  = fmat4_mul(V, M);
    const fmat4_t MVP = fmat4_mul(VP, M);

    glUniformMatrix4fv(glGetUniformLocation(program.id, "MV"),  1, GL_TRUE, (const float *)&MV);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "MVP"), 1, GL_TRUE, (const float *)&MVP);

    block_mesh_render(&render_info_nodes[i].value.transparent_mesh);
  }
}

uint32_t block_renderer_get_texture_index(voxy_block_id_t id, direction_t direction)
{
  return texture_indices[id][direction];
}
