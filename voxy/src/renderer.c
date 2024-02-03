#include "renderer.h"

#include "check.h"

#include <stdio.h>
#include <stdlib.h>

int renderer_init(struct renderer *renderer)
{
  VOXY_CHECK_DECLARE(chunk_program);
  VOXY_CHECK_DECLARE(ui);

  VOXY_CHECK_INIT(chunk_program, gl_program_load(&renderer->chunk_program, 2, (GLenum[]){GL_VERTEX_SHADER, GL_FRAGMENT_SHADER}, (const char *[]){"assets/chunk.vert", "assets/chunk.frag"}));
  VOXY_CHECK_INIT(ui,            ui_init(&renderer->ui));

  return 0;

error:
  VOXY_CHECK_FINI(chunk_program, gl_program_fini(&renderer->chunk_program));
  VOXY_CHECK_FINI(ui,            ui_fini(&renderer->ui));
  return -1;
}

void renderer_fini(struct renderer *renderer)
{
  gl_program_fini(&renderer->chunk_program);
  ui_fini(&renderer->ui);
}

void renderer_render(struct renderer *renderer, int width, int height, struct resource_pack *resource_pack, struct camera *camera, struct world *world)
{
  glViewport(0, 0, width, height);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  renderer_render_chunks(renderer, camera, resource_pack, world);
  renderer_render_ui(renderer, width, height, resource_pack, world);
}
