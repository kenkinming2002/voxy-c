#include "renderer_world.h"

#include <types/chunk_mesh.h>
#include <types/chunk.h>
#include <types/entity.h>
#include <types/player.h>
#include <types/world.h>
#include <types/mod_assets.h>

#include <voxy/mod_interface.h>

#include <core/window.h>
#include "camera.h"
#include "check.h"

int renderer_world_init(struct renderer_world *renderer_world)
{
  VOXY_CHECK_DECLARE(program_chunk);
  VOXY_CHECK_DECLARE(program_outline);

  VOXY_CHECK_INIT(program_chunk,   gl_program_load(&renderer_world->program_chunk,   2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/chunk.vert",   "assets/chunk.frag"}));
  VOXY_CHECK_INIT(program_outline, gl_program_load(&renderer_world->program_outline, 2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/outline.vert", "assets/outline.frag"}));

  return 0;

error:
  VOXY_CHECK_FINI(program_chunk,   gl_program_fini(&renderer_world->program_chunk));
  VOXY_CHECK_FINI(program_outline, gl_program_fini(&renderer_world->program_outline));
  return -1;
}

void renderer_world_fini(struct renderer_world *renderer_world)
{
  gl_program_fini(&renderer_world->program_chunk);
  gl_program_fini(&renderer_world->program_outline);
}

void renderer_world_render(struct renderer_world *renderer_world, struct world *world, struct mod *mod, struct mod_assets *mod_assets)
{
  struct camera camera;
  camera.transform = entity_view_transform(&world->player.base);
  camera.fovy   = M_PI / 2.0f;
  camera.near   = 0.1f;
  camera.far    = 1000.0f;
  camera.aspect = (float)window_size.x / (float)window_size.y;
  if(world->player.third_person)
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

  glUseProgram(renderer_world->program_chunk.id);
  glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_chunk.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_chunk.id, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mod_assets->block_array_texture.id);

  for(size_t i=0; i<world->chunks.bucket_count; ++i)
    for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
      if(chunk->chunk_mesh)
      {
        glBindVertexArray(chunk->chunk_mesh->vao_opaque);
        glDrawElements(GL_TRIANGLES, chunk->chunk_mesh->count_opaque, GL_UNSIGNED_INT, 0);
      }

  for(size_t i=0; i<world->chunks.bucket_count; ++i)
    for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
      if(chunk->chunk_mesh)
      {
        glBindVertexArray(chunk->chunk_mesh->vao_transparent);
        glDrawElements(GL_TRIANGLES, chunk->chunk_mesh->count_transparent, GL_UNSIGNED_INT, 0);
      }

  ivec3_t position;
  ivec3_t normal;
  bool hit = entity_ray_cast(&world->player.base, world, mod, 20.0f, &position, &normal);

  if(hit || world->player.third_person)
  {
    glUseProgram(renderer_world->program_outline.id);
    glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_outline.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    if(hit)
    {
      ivec3_t position_destroy = position;
      ivec3_t position_place   = ivec3_add(position, normal);

      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"), position_destroy.x, position_destroy.y, position_destroy.z);
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "dimension"), 1.0f, 1.0f, 1.0f);
      glUniform4f(glGetUniformLocation(renderer_world->program_outline.id, "color"),     1.0f, 1.0f, 1.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);

      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"), position_place.x, position_place.y, position_place.z);
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "dimension"), 1.0f, 1.0f, 1.0f);
      glUniform4f(glGetUniformLocation(renderer_world->program_outline.id, "color"),     1.0f, 1.0f, 1.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);
    }

    if(world->player.third_person)
    {
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"),  world->player.base.position.x,  world->player.base.position.y,  world->player.base.position.z);
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "dimension"), world->player.base.dimension.x, world->player.base.dimension.y, world->player.base.dimension.z);
      glUniform4f(glGetUniformLocation(renderer_world->program_outline.id, "color"),     1.0f, 0.0f, 0.0f, 1.0f);
      glDrawArrays(GL_LINES, 0, 24);
    }
  }
}
