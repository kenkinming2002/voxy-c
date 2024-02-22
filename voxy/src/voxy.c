#include <voxy/main_game/main_game.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/config.h>

#include <voxy/core/window.h>
#include <voxy/core/delta_time.h>
#include <voxy/core/log.h>

#include <stdlib.h>

#define FIXED_DT (1.0f/30.0f)

int main()
{
  float accumulated_dt = 0.0f;

  window_init(WINDOW_TITLE, WINDOW_WIDTH, WINDOW_HEIGHT);
  mod_load(MOD_FILEPATH);
  while(!window_should_close())
  {
    accumulated_dt += get_delta_time();

    if(accumulated_dt >= FIXED_DT)
    {
      window_update();
      main_game_update(FIXED_DT);
      accumulated_dt -= FIXED_DT;
    }

    if(accumulated_dt >= FIXED_DT)
    {
      LOG_WARN("Skipping %d fixed update", (int)floorf(accumulated_dt / FIXED_DT));
      accumulated_dt = fmodf(accumulated_dt, FIXED_DT);
    }

    main_game_render();
    window_present();
  }
}

