#include "blocks.h"
#include "entities.h"
#include "debug.h"

#include <libcommon/core/window.h>
#include <libcommon/graphics/render.h>
#include <libcommon/ui/ui.h>

void main_game_render(void)
{
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glLineWidth(3.0f);

  glViewport(0, 0, window_size.x, window_size.y);
  glClearColor(0.52f, 0.81f, 0.98f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  main_game_render_blocks();
  main_game_render_entities();
  main_game_render_debug();

  render_end();
  ui_render();
}

