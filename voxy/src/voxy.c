#include <voxy/scene/scene.h>
#include <voxy/core/window.h>

#define WINDOW_WIDTH  1024
#define WINDOW_HEIGHT 720
#define WINDOW_TITLE "voxy"

int main()
{
  // Window always exist
  window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);

  // By default we enter are in main menu state
  scene_switch(SCENE_MAIN_GAME);
  scene_commit();

  // Main loop
  while(!window_should_close())
  {
    scene_update();
    scene_render();
    window_present();
    if(scene_commit())
      break;
  }
}

