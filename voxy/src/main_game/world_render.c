#include <voxy/main_game/world_render.h>

#include <voxy/main_game/assets.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/chunk.h>

#include <voxy/core/window.h>

#include <voxy/graphics/gl.h>
#include <voxy/graphics/camera.h>

#include <stdio.h>
#include <stdlib.h>

void world_render()
{
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  struct player *player = player_get();
  if(!player)
    return;

  struct camera camera = player_get_camera(player);

  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(&camera), V);

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(3.0f);

  // 1: Chunk Rendering
  {
    world_chunk_for_each(chunk)
    {
      fvec3_t min = fvec3(+INFINITY, +INFINITY, +INFINITY);
      fvec3_t max = fvec3(-INFINITY, -INFINITY, -INFINITY);
      for(int k=0; k<2; ++k)
        for(int j=0; j<2; ++j)
          for(int i=0; i<2; ++i)
          {
            fvec3_t point_world_space = fvec3_sub(ivec3_as_fvec3(ivec3_mul_scalar(ivec3_add(chunk->position, ivec3(i, j, k)), CHUNK_WIDTH)), fvec3(0.5f, 0.5f, 0.5f));
            fvec3_t point_clip_space  = fmat4_apply_fvec3_perspective_divide(VP, point_world_space);

            min = fvec3_min(min, point_clip_space);
            max = fvec3_max(max, point_clip_space);
          }

      chunk->culled = (min.x >= 1.0f || max.x <= -1.0f)
        && (min.y >= 1.0f || max.y <= -1.0f)
        && (min.z >= 1.0f || max.z <= -1.0f);
    }

    struct gl_program program = GL_PROGRAM_LOAD(chunk);
    glUseProgram(program.id);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "V"),  1, GL_TRUE, (const float *)&V);
    glBindTexture(GL_TEXTURE_2D_ARRAY, assets_get_block_texture_array().id);

    world_chunk_for_each(chunk)
      if(!chunk->culled && chunk->count_opaque != 0)
      {
        glBindVertexArray(chunk->vao_opaque);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, chunk->count_opaque);
      }

    world_chunk_for_each(chunk)
      if(!chunk->culled && chunk->count_transparent != 0)
      {
        glBindVertexArray(chunk->vao_transparent);
        glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0, chunk->count_transparent);
      }
  }

  // 2: Entity outline rendering
  {
    struct gl_program program = GL_PROGRAM_LOAD(outline);
    glUseProgram(program.id);
    glUniformMatrix4fv(glGetUniformLocation(program.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    world_chunk_for_each(chunk)
      for(size_t i=0; i<chunk->entity_count; ++i)
      {
        if(chunk->entities[i] == player_as_entity(player) && !player_get_third_person(player))
          continue;

        glUniform3f(glGetUniformLocation(program.id, "position"),  chunk->entities[i]->position.x,  chunk->entities[i]->position.y,  chunk->entities[i]->position.z);
        glUniform3f(glGetUniformLocation(program.id, "dimension"), chunk->entities[i]->dimension.x, chunk->entities[i]->dimension.y, chunk->entities[i]->dimension.z);
        glUniform4f(glGetUniformLocation(program.id, "color"),     1.0f, 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_LINES, 0, 24);
      }
  }
}
