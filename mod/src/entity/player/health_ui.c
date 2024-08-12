#include "health_ui.h"

#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/ui.h>
#include <libcommon/core/window.h>
#include <libcommon/utils/utils.h>

static struct gl_texture_2d get_heart_full_texture(void)
{
  static int initialized = false;
  static struct gl_texture_2d texture = {0};
  if(!initialized)
  {
    gl_texture_2d_load(&texture, "mod/assets/textures/heart_full.png");
    initialized = true;
  }
  return texture;
}

static struct gl_texture_2d get_heart_empty_texture(void)
{
  static int initialized = false;
  static struct gl_texture_2d texture = {0};
  if(!initialized)
  {
    gl_texture_2d_load(&texture, "mod/assets/textures/heart_empty.png");
    initialized = true;
  }
  return texture;
}


void player_entity_update_health_ui(struct entity *entity)
{
  const unsigned i_max_health = floorf(entity->max_health);
  const unsigned i_health = floorf(entity->max_health);

  const float base = MIN(window_size.x, window_size.y);
  const float margin = base * 0.008f;
  const float cell_width = base * 0.05f;
  const float cell_sep = base * 0.006f;

  const float hotbar_width = cell_sep + PLAYER_HOTBAR_SIZE * (cell_sep + cell_width);
  const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);

  const fvec2_t health_bar_position = fvec2(hotbar_position.x, margin + cell_sep + cell_width + cell_sep);
  const fvec2_t heart_dimension = fvec2(base * 0.025f, base * 0.025f);

  const struct gl_texture_2d heart_full_texture = get_heart_full_texture();
  const struct gl_texture_2d heart_empty_texture = get_heart_empty_texture();
  for(unsigned i=0; i<i_max_health; ++i)
  {
    fvec2_t heart_position = health_bar_position;
    heart_position.x += i * heart_dimension.x;

    const struct gl_texture_2d texture = i < i_health ? heart_full_texture : heart_empty_texture;
    ui_rect_textured(heart_position, heart_dimension, 0.0f, texture.id);
  }
}
