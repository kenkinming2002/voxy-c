#include <voxy/scene/scene.h>

#include <voxy/scene/main_menu/main_menu.h>
#include <voxy/scene/main_game/main_game.h>

static enum Scene current_scene = SCENE_NONE;
static enum Scene next_scene = SCENE_NONE;

void scene_switch(enum Scene scene)
{
  next_scene = scene;
}

void scene_update(void)
{
  switch(current_scene)
  {
  case SCENE_MAIN_MENU:
    main_menu_update();
    break;
  case SCENE_MAIN_GAME:
    main_game_update();
    break;
  case SCENE_NONE:
    break;
  }
}

void scene_render(void)
{
  switch(current_scene)
  {
  case SCENE_MAIN_MENU:
    main_menu_render();
    break;
  case SCENE_MAIN_GAME:
    main_game_render();
    break;
  case SCENE_NONE:
    break;
  }
}

void scene_commit(void)
{
  if(current_scene != next_scene)
  {
    switch(current_scene)
    {
      case SCENE_MAIN_MENU:
        main_menu_leave();
        break;
      case SCENE_MAIN_GAME:
        main_game_leave();
        break;
      case SCENE_NONE:
        break;
    }

    switch(next_scene)
    {
      case SCENE_MAIN_MENU:
        main_menu_enter();
        break;
      case SCENE_MAIN_GAME:
        main_game_enter();
        break;
      case SCENE_NONE:
        break;
    }

    current_scene = next_scene;
  }
}
