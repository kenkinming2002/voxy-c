#include "renderer.h"

#include "gl.h"

#include <stdio.h>
#include <stdlib.h>

int renderer_init(struct renderer *renderer, struct resource_pack *resource_pack)
{
  size_t       filepath_count;
  const char **filepaths = NULL;
  bool ui_initialized = false;

  renderer->chunk_program             = 0;
  renderer->chunk_block_texture_array = 0;

  if((renderer->chunk_program = gl_program_load("assets/chunk.vert", "assets/chunk.frag")) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load chunk shader\n");
    goto error;
  }

  filepath_count = resource_pack->block_texture_info_count;
  filepaths      = malloc(filepath_count * sizeof *filepaths);
  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;

  if((renderer->chunk_block_texture_array = gl_array_texture_load(filepath_count, filepaths)) == 0)
  {
    fprintf(stderr, "ERROR: Failed to load block textures\n");
    goto error;
  }

  free(filepaths);

  if(ui_init(&renderer->ui) != 0)
  {
    fprintf(stderr, "ERROR: Failed to initialize ui\n");
    goto error;
  }
  ui_initialized = true;

  font_set_init(&renderer->font_set);
  font_set_load(&renderer->font_set, "assets/arial.ttf");
  font_set_load(&renderer->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&renderer->font_set);

  return 0;

error:
  if(ui_initialized) ui_fini(&renderer->ui);
  if(renderer->chunk_program             != 0) glDeleteProgram(renderer->chunk_program);
  if(renderer->chunk_block_texture_array != 0) glDeleteTextures(1, &renderer->chunk_block_texture_array);
  if(filepaths)
    free(filepaths);
  return -1;
}

void renderer_fini(struct renderer *renderer)
{
  font_set_fini(&renderer->font_set);
  ui_fini(&renderer->ui);
  glDeleteProgram(renderer->chunk_program);
  glDeleteTextures(1, &renderer->chunk_block_texture_array);
}

void renderer_render(struct renderer *renderer, struct window *window, struct camera *camera, struct world *world)
{
  int width, height;
  window_get_framebuffer_size(window, &width, &height);

  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_render_chunks(renderer, camera, world);
  renderer_render_ui(renderer, window, world);
}
