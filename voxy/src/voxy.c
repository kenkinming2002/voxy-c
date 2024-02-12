#include <main_game/main_game.h>
#include <main_game/mod.h>

#include <core/window.h>
#include <core/delta_time.h>

#include <config.h>
#include <stdlib.h>

int main()
{
  window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
  mod_load(MOD_FILEPATH);
  while(!window_should_close())
  {
    float dt = get_delta_time();

    window_begin();
    main_game_update(dt);
    main_game_render();
    window_end();
  }
}

