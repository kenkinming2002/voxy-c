#include <voxy/scene/main_game/render/debug_overlay.h>
#include <voxy/scene/main_game/render/debug.h>

#include <libcommon/utils/dynamic_array.h>
#include <libcommon/ui/ui.h>
#include <libcommon/core/window.h>

#include <stdarg.h>
#include <stdio.h>

static DYNAMIC_ARRAY_DECLARE(messages, char *);

void main_game_reset_debug_overlay(void)
{
  for(size_t i=0; i<messages.item_count; ++i)
    free(messages.items[i]);
  messages.item_count = 0;
}

void main_game_debug_overlay_printf(const char *restrict format, ...)
{
  char *message;

  va_list ap;
  va_start(ap, format);
  vasprintf(&message, format, ap);
  va_end(ap);

  DYNAMIC_ARRAY_APPEND(messages, message);
}

static float text_factor()
{
  return window_size.x < window_size.y ? window_size.x : window_size.y;
}

static float text_margin(void)
{
  return text_factor() * 0.005f;
}

static float text_height(void)
{
  return text_factor() * 0.02f;
}

void main_game_render_debug_overlay(void)
{
  if(!main_game_render_get_debug())
    return;

  const float margin = text_margin();
  const float height = text_height();

  float y = (float)window_size.y - margin - height;
  for(size_t i=0; i<messages.item_count; ++i)
  {
    ui_text(fvec2(margin, y), height, 1.0f, messages.items[i]);
    y -= height;
    y -= margin;
  }
}

