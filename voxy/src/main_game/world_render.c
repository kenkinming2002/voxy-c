#include <voxy/main_game/world_render.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod_assets.h>

#include <voxy/core/window.h>

#include <voxy/graphics/gl.h>
#include <voxy/graphics/gl_programs.h>
#include <voxy/graphics/camera.h>

#include <voxy/types/chunk.h>
#include <voxy/types/player.h>

#include <stdio.h>
#include <stdlib.h>

void world_render()
{
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  struct gl_program *program_chunk   = gl_program_chunk_get();
  struct gl_program *program_outline = gl_program_outline_get();

  struct player *player = player_get();
  if(!player)
    return;

  struct entity *player_entity = player_as_entity(player);

  struct camera camera;
  camera.transform = entity_view_transform(player_entity);
  camera.fovy   = M_PI / 2.0f;
  camera.near   = 0.1f;
  camera.far    = 1000.0f;
  camera.aspect = (float)window_size.x / (float)window_size.y;
  if(player_third_person(player))
    camera.transform = transform_local_translate(camera.transform, fvec3(0.0f, -10.0f, 0.0f));

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

  glUseProgram(program_chunk->id);
  glUniformMatrix4fv(glGetUniformLocation(program_chunk->id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(program_chunk->id, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mod_assets_block_array_texture_get()->id);

  world_chunk_for_each(chunk)
  {
    glBindVertexArray(chunk->vao_opaque);
    glDrawElements(GL_TRIANGLES, chunk->count_opaque, GL_UNSIGNED_INT, 0);
  }

  world_chunk_for_each(chunk)
  {
    glBindVertexArray(chunk->vao_transparent);
    glDrawElements(GL_TRIANGLES, chunk->count_transparent, GL_UNSIGNED_INT, 0);
  }

  ivec3_t position;
  ivec3_t normal;
  bool hit = entity_ray_cast(player_entity, 20.0f, &position, &normal);

  if(hit || player_third_person(player) || entity_count != 0)
  {
    glUseProgram(program_outline->id);
    glUniformMatrix4fv(glGetUniformLocation(program_outline->id, "VP"), 1, GL_TRUE, (const float *)&VP);

    if(hit)
    {
      ivec3_t position_destroy = position;
      ivec3_t position_place   = ivec3_add(position, normal);

      glUniform3f(glGetUniformLocation(program_outline->id, "position"), position_destroy.x, position_destroy.y, position_destroy.z);
      glUniform3f(glGetUniformLocation(program_outline->id, "dimension"), 1.0f, 1.0f, 1.0f);
      glUniform4f(glGetUniformLocation(program_outline->id, "color"),     1.0f, 1.0f, 1.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);

      glUniform3f(glGetUniformLocation(program_outline->id, "position"), position_place.x, position_place.y, position_place.z);
      glUniform3f(glGetUniformLocation(program_outline->id, "dimension"), 1.0f, 1.0f, 1.0f);
      glUniform4f(glGetUniformLocation(program_outline->id, "color"),     1.0f, 1.0f, 1.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);
    }

    for(size_t i=0; i<entity_count; ++i)
    {
      if(entities[i] == player_entity && !player_third_person(player))
        continue;

      glUniform3f(glGetUniformLocation(program_outline->id, "position"),  entities[i]->position.x,  entities[i]->position.y,  entities[i]->position.z);
      glUniform3f(glGetUniformLocation(program_outline->id, "dimension"), entities[i]->dimension.x, entities[i]->dimension.y, entities[i]->dimension.z);
      glUniform4f(glGetUniformLocation(program_outline->id, "color"),     1.0f, 0.0f, 0.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);
    }
  }
}
