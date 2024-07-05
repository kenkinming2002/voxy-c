#include <voxy/scene/main_menu/main_menu.h>

#include <voxy/scene/scene.h>

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
  if(input_press(KEY_ESC))
    scene_switch(SCENE_MAIN_GAME);
}

void main_menu_render(void)
{
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
