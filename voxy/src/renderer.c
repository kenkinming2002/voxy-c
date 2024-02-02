#include "renderer.h"

#include "check.h"

#include <stdio.h>
#include <stdlib.h>

int renderer_init(struct renderer *renderer, struct resource_pack *resource_pack)
{
  size_t       filepath_count = resource_pack->block_texture_info_count;
  const char **filepaths      = malloc(filepath_count * sizeof *filepaths);
  for(size_t i=0; i<filepath_count; ++i)
    filepaths[i] = resource_pack->block_texture_infos[i].filepath;

  VOXY_CHECK_DECLARE(chunk_program);
  VOXY_CHECK_DECLARE(chunk_array_texture_2d);
  VOXY_CHECK_DECLARE(ui);

  VOXY_CHECK_INIT(chunk_program,          gl_program_load(&renderer->chunk_program, 2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/chunk.vert", "assets/chunk.frag"}));
  VOXY_CHECK_INIT(chunk_array_texture_2d, gl_array_texture_2d_load(&renderer->chunk_array_texture_2d, filepath_count, filepaths));
  VOXY_CHECK_INIT(ui,                     ui_init(&renderer->ui));

  font_set_init(&renderer->font_set);
  font_set_load(&renderer->font_set, "assets/arial.ttf");
  font_set_load(&renderer->font_set, "/usr/share/fonts/noto/NotoColorEmoji.ttf");
  font_set_load_system(&renderer->font_set);

  free(filepaths);
  return 0;

error:
  VOXY_CHECK_FINI(chunk_program,          gl_program_fini(&renderer->chunk_program));
  VOXY_CHECK_FINI(chunk_array_texture_2d, gl_array_texture_2d_fini(&renderer->chunk_array_texture_2d));
  VOXY_CHECK_FINI(ui,                     ui_fini(&renderer->ui));
  free(filepaths);
  return -1;
}

void renderer_fini(struct renderer *renderer)
{
  gl_program_fini(&renderer->chunk_program);
  gl_array_texture_2d_fini(&renderer->chunk_array_texture_2d);
  ui_fini(&renderer->ui);
  font_set_fini(&renderer->font_set);
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
