#include <voxy/scene/main_game/render/debug.h>

#include "debug_gizmos.h"
#include "debug_overlay.h"

static bool g_debug;

void main_game_render_set_debug(bool debug)
{
  g_debug = debug;
}

bool main_game_render_get_debug(void)
{
  return g_debug;
}

void main_game_render_debug(void)
{
  if(main_game_render_get_debug())
  {
    main_game_render_debug_gizmos();
    main_game_render_debug_overlay();
  }
}
