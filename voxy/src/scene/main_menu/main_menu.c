#include <voxy/scene/main_menu/main_menu.h>

#include <voxy/scene/scene.h>

#include <libcommon/graphics/ui.h>
#include <libcommon/core/window.h>

#include <glad/glad.h>

void main_menu_enter(void)
{
}

void main_menu_leave(void)
{
}

void main_menu_update(void)
{
  window_update();
  ui_reset();

  // Again I am laying out UI component manually. Sue me.
  const unsigned int height = 100;
  const float margin = 5.0f;

  const char *text1 = "Exit";
  const char *text2 = "Play";

  const float width1 = ui_text_width(height, text1);
  const float width2 = ui_text_width(height, text2);

  const fvec2_t position1 = fvec2(((float)window_size.x - width1) * 0.5f, ((float)window_size.y + margin) * 0.5f - (float)height);
  const fvec2_t position2 = fvec2(((float)window_size.x - width2) * 0.5f, ((float)window_size.y + margin) * 0.5f);

  const fvec2_t dimension1 = fvec2(width1, height);
  const fvec2_t dimension2 = fvec2(width2, height);

  enum ui_button_result result;

  result = ui_button(position1, dimension1);
  if(result & UI_BUTTON_RESULT_HOVERED)
    ui_quad_colored(position1, dimension1, 0.1f, fvec4(0.3f, 0.3f, 0.3f, 1.0f));

  ui_text(position1, height, text1);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT)
    scene_switch(SCENE_EXIT);

  result = ui_button(position2, dimension2);
  if(result & UI_BUTTON_RESULT_HOVERED)
    ui_quad_colored(position2, dimension2, 0.1f, fvec4(0.3f, 0.3f, 0.3f, 1.0f));

  ui_text(position2, height, text2);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT)
    scene_switch(SCENE_MAIN_GAME);
}

void main_menu_render(void)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ui_render();
}
