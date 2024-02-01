#include "world_renderer.h"

#include <voxy/math/vector.h>

#include "gl.h"
#include "config.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int world_renderer_init(struct world_renderer *world_renderer, struct resource_pack *resource_pack)
{
  world_renderer->chunk_program             = 0;
  world_renderer->chunk_block_texture_array = 0;

  if((world_renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load chunk shader\n");
    goto error;
  }

  size_t       filepath_count = resource_pack->block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);

  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;
  world_renderer->chunk_block_texture_array = gl_array_texture_load(filepath_count, filepaths);

  free(filepaths);

  if(world_renderer->chunk_block_texture_array == 0)
  {
    fprintf(stderr, "ERROR: Failed to load block textures\n");
    goto error;
  }

  return 0;

error:
  if(world_renderer->chunk_program != 0)
    glDeleteProgram(world_renderer->chunk_program);

  if(world_renderer->chunk_block_texture_array != 0)
    glDeleteTextures(1, &world_renderer->chunk_block_texture_array);

  return -1;
}

void world_renderer_fini(struct world_renderer *world_renderer)
{
  glDeleteProgram(world_renderer->chunk_program);
  glDeleteTextures(1, &world_renderer->chunk_block_texture_array);
}


void world_renderer_render(struct world_renderer *world_renderer, struct world *world, struct camera *camera)
{
  fmat4_t VP = fmat4_identity();
  VP = fmat4_mul(camera_view_matrix(camera),       VP);
  VP = fmat4_mul(camera_projection_matrix(camera), VP);

  fmat4_t V = fmat4_identity();
  V = fmat4_mul(camera_view_matrix(camera), V);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glUseProgram(world_renderer->chunk_program);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "VP"), 1, GL_TRUE, (const float *)&VP);
  glUniformMatrix4fv(glGetUniformLocation(world_renderer->chunk_program, "V"),  1, GL_TRUE, (const float *)&V);
  glBindTexture(GL_TEXTURE_CUBE_MAP, world_renderer->chunk_block_texture_array);

  for(size_t i=0; i<world->chunks.bucket_count; ++i)
    for(struct chunk *chunk = world->chunks.buckets[i].head; chunk; chunk = chunk->next)
      if(chunk->chunk_mesh)
      {
        glBindVertexArray(chunk->chunk_mesh->vao);
        glDrawElements(GL_TRIANGLES, chunk->chunk_mesh->count, GL_UNSIGNED_INT, 0);
      }

  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
}

