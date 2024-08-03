#include <voxy/scene/main_game/render/blocks.h>
#include <voxy/scene/main_game/render/blocks_render_info.h>

#include <voxy/scene/main_game/render/assets.h>
#include <voxy/scene/main_game/render/debug.h>

#include <voxy/scene/main_game/states/camera.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/digger.h>

#include <voxy/graphics/camera.h>
#include <voxy/graphics/gl.h>

#include <voxy/dynamic_array.h>

#include <time.h>

static struct blocks_render_info_hash_table blocks_render_infos;

/// Dispose of render info for chunks that are outside of render distance.
static void dispose_render_infos(ivec3_t center, int distance)
{
  struct blocks_render_info **blocks_render_info;
  SC_HASH_TABLE_FOREACH_P(blocks_render_infos, blocks_render_info)
  {
    const ivec3_t position = (*blocks_render_info)->position;
    if(position.z < center.z - distance + 1 || position.z > center.z + distance - 1 ||
       position.y < center.y - distance + 1 || position.y > center.y + distance - 1 ||
       position.x < center.x - distance + 1 || position.x > center.x + distance - 1)
    {
      struct blocks_render_info *old_render_info = *blocks_render_info;
      *blocks_render_info = old_render_info->next;
      blocks_render_infos.load -= 1;
      blocks_render_info_destroy(old_render_info);
      if(!*blocks_render_info)
        break;
    }
  }
}

/// Check if there is any chunks within render distance where either their
/// render infos have not been created yet or need to be updated.
static void update_render_infos(ivec3_t center, int distance, size_t *count)
{
  for(int z = center.z - distance + 1; z <= center.z + distance - 1; ++z)
    for(int y = center.y - distance + 1; y <= center.y + distance - 1; ++y)
      for(int x = center.x - distance + 1; x <= center.x + distance - 1; ++x)
      {
        const ivec3_t chunk_position = ivec3(x, y, z);
        struct chunk *chunk = chunk_hash_table_lookup(&world_chunks, chunk_position);
        if(chunk && chunk->data)
        {
          struct blocks_render_info *blocks_render_info = blocks_render_info_hash_table_lookup(&blocks_render_infos, chunk_position);
          if(!blocks_render_info)
          {
            blocks_render_info = blocks_render_info_create();
            blocks_render_info->position = chunk_position;
            blocks_render_info_hash_table_insert(&blocks_render_infos, blocks_render_info);
            blocks_render_info_update(blocks_render_info, chunk, &g_digger);

            *count += 1;
            chunk->mesh_invalidated = false;
          }
          else if(chunk->mesh_invalidated)
          {
            blocks_render_info_update(blocks_render_info, chunk, &g_digger);

            *count += 1;
            chunk->mesh_invalidated = false;
          }
        }
      }
}

/// Rendering.
static void render(const struct camera *camera)
{
  struct blocks_render_info *blocks_render_info;

  SC_HASH_TABLE_FOREACH(blocks_render_infos, blocks_render_info)
    blocks_render_info_cull(blocks_render_info, camera);

  blocks_render_info_render_begin(camera);

  SC_HASH_TABLE_FOREACH(blocks_render_infos, blocks_render_info)
    blocks_render_info_render_opaque(blocks_render_info);

  SC_HASH_TABLE_FOREACH(blocks_render_infos, blocks_render_info)
    blocks_render_info_render_transparent(blocks_render_info);

  // FIXME: We are loadiing the same gl program twice
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&world_camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&world_camera), VP);

  struct gl_program program = GL_PROGRAM_LOAD(outline);
  glUseProgram(program.id);
  glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

  if(main_game_render_get_debug())
    SC_HASH_TABLE_FOREACH(blocks_render_infos, blocks_render_info)
      if(!blocks_render_info->culled)
      {
        fvec3_t chunk_position = fvec3_add_scalar(ivec3_as_fvec3(ivec3_mul_scalar(blocks_render_info->position, CHUNK_WIDTH)), (CHUNK_WIDTH-1) * 0.5f);
        fvec3_t chunk_dimension = fvec3(CHUNK_WIDTH, CHUNK_WIDTH, CHUNK_WIDTH);

        glUniform3f(glGetUniformLocation(program.id, "position"), chunk_position.x, chunk_position.y, chunk_position.z);
        glUniform3f(glGetUniformLocation(program.id, "dimension"), chunk_dimension.x, chunk_dimension.y, chunk_dimension.z);
        glUniform4f(glGetUniformLocation(program.id, "color"), 1.0f, 1.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 24);
      }
}

void main_game_render_blocks(void)
{
  size_t count = 0;
  clock_t begin = clock();

  const ivec3_t center = get_chunk_position_f(world_camera.transform.translation);
  const int distance = 8;

  dispose_render_infos(center, distance);
  update_render_infos(center, distance, &count);

  clock_t end = clock();
  if(count != 0)
    LOG_INFO("Chunk System: Remeshed %zu chunks in %fs - Average %fs", count, (float)(end - begin) / (float)CLOCKS_PER_SEC, (float)(end - begin) / (float)CLOCKS_PER_SEC / (float)count);

  render(&world_camera);
}

