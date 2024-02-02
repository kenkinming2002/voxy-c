#include "renderer.h"

void renderer_render_chunks(struct renderer *renderer, struct camera *camera, struct world *world)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glUseProgram(renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(renderer->chunk_program, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_2D_ARRAY, renderer->chunk_block_texture_array);

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

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}
