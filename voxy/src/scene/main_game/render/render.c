#include <voxy/scene/main_game/render/render.h>

#include <voxy/scene/main_game/render/blocks.h>
#include <voxy/scene/main_game/render/entities.h>
#include <voxy/scene/main_game/render/debug.h>

#include <voxy/core/window.h>
#include <voxy/graphics/ui.h>

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

  ui_render();
}

