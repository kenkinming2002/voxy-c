#include "renderer_world.h"

#include "window.h"
#include "camera.h"
#include "check.h"
#include "resource_pack.h"
#include "world.h"

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

void renderer_world_render(struct renderer_world *renderer_world, struct window *window, struct world *world, struct resource_pack *resource_pack)
{
  struct camera camera;
  camera.transform = world->player.transform;
  camera.fovy      = M_PI / 2.0f;
  camera.near      = 1.0f;
  camera.far       = 1000.0f;
  camera.aspect    = (float)window->width / (float)window->height;

  if(world->player.third_person)
    transform_local_translate(&camera.transform, fvec3(0.0f, -30.0f, 0.0f));

  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(&camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(&camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(&camera), V);

  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glLineWidth(5.0f);

  glUseProgram(renderer_world->program_chunk.id);
  glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_chunk.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_chunk.id, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, resource_pack->block_array_texture.id);

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

  if(world->player.has_target_place || world->player.has_target_destroy)
  {
    glUseProgram(renderer_world->program_outline.id);
    glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_outline.id, "VP"), 1, GL_TRUE, (const float *)&VP);

    if(world->player.has_target_destroy)
    {
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"), world->player.target_destroy.x, world->player.target_destroy.y, world->player.target_destroy.z);
      glDrawArrays(GL_LINES, 0, 24);
    }

    if(world->player.has_target_destroy &&world->player.has_target_place)
    {
      glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"), world->player.target_place.x, world->player.target_place.y, world->player.target_place.z);
      glDrawArrays(GL_LINES, 0, 24);
    }
  }
}
