#include "renderer_world.h"

#include "camera.h"
#include "resource_pack.h"
#include "world.h"
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

void renderer_world_render(struct renderer_world *renderer_world, int width, int height, struct world *world, struct resource_pack *resource_pack)
{
  struct camera camera;
  camera.transform = world->player.transform;
  camera.fovy      = M_PI / 2.0f;
  camera.near      = 1.0f;
  camera.far       = 1000.0f;
  camera.aspect    = (float)width / (float)height;

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

  glUseProgram(renderer_world->program_outline.id);
  glUniformMatrix4fv(glGetUniformLocation(renderer_world->program_outline.id, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniform3f(glGetUniformLocation(renderer_world->program_outline.id, "position"), 0.0f, 0.0f, 10.0f);

  glDrawArrays(GL_LINES, 0, 24);
}
