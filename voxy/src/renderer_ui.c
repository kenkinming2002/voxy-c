#include "renderer.h"

static inline float minf(float a, float b)
{
  return a < b ? a : b;
}

void renderer_render_ui(struct renderer *renderer, int width, int height, struct resource_pack *resource_pack, struct world *world)
{
  ui_begin(&renderer->ui, fvec2(width, height));

  const int count = 9;

  const float sep               = minf(width, height) * 0.006f;
  const float inner_width       = minf(width, height) * 0.05f;
  const float outer_width       = inner_width + 2.0f * sep;
  const float total_width       = count * inner_width + (count + 1) * sep;
  const float total_height      = outer_width;
  const float margin_horizontal = (width - total_width) * 0.5f;
  const float margin_vertical   = height * 0.03f;

  char buffer[32];

  snprintf(buffer, sizeof buffer, "Selected %d 你好 😀", world->player.selection);
  ui_draw_text_centered(&renderer->ui, &resource_pack->font_set, fvec2(width * 0.5f, margin_vertical + outer_width + sep), buffer, 24);
  ui_draw_quad_rounded(&renderer->ui, fvec2(margin_horizontal, margin_vertical), fvec2(total_width, total_height), sep, fvec4(0.9f, 0.9f, 0.9f, 0.3f));
  for(int i=0; i<count; ++i)
  {
    fvec4_t color = i + 1 == world->player.selection ? fvec4(0.95f, 0.75f, 0.75f, 0.8f) : fvec4(0.95f, 0.95f, 0.95f, 0.7f);
    ui_draw_quad_rounded(&renderer->ui, fvec2(margin_horizontal + i * inner_width + (i + 1) * sep, margin_vertical + sep), fvec2(inner_width, inner_width), sep, color);
  }
}
