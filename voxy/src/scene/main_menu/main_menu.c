#include <voxy/scene/main_menu/main_menu.h>

#include <voxy/scene/scene.h>

#include <voxy/graphics/ui.h>
#include <voxy/core/window.h>

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

  // Play button
  {
    const unsigned int height = 30;
    const char *text = "Play";

    const fvec2_t dimension = fvec2(ui_text_width(height, text), height);
    const fvec2_t position = fvec2_mul_scalar(fvec2_sub(ivec2_as_fvec2(window_size), dimension), 0.5f);

    enum ui_button_result result = ui_button(position, dimension);
    if(result & UI_BUTTON_RESULT_HOVERED)
      ui_quad_colored(position, dimension, 0.1f, fvec4(0.3f, 0.3f, 0.3f, 1.0f));

    ui_text(position, height, text);
    if(result & UI_BUTTON_RESULT_CLICK_LEFT)
      scene_switch(SCENE_MAIN_GAME);
  }
}

void main_menu_render(void)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  ui_render();
}
