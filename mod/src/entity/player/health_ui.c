#include "health_ui.h"

#include "entity/item/item.h"

#include <voxy/scene/main_game/states/chunks.h>

#include <libcommon/graphics/gl.h>
#include <libcommon/ui/ui.h>
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

static inline float rand_range(float a, float b)
{
  return a + ((float)rand() / (float)RAND_MAX) * (b - a);
}

static inline fvec3_t random_velocity(void)
{
  fvec3_t result;
  do {
    result.x = rand_range(-1.0f, 1.0f);
    result.y = rand_range(-1.0f, 1.0f);
  } while(result.x * result.x + result.y * result.y > 1.0f);
  result.z = rand_range(0.0f, 1.0f);
  return fvec3_mul_scalar(result, 1.0f);
}

static inline void spill_item(fvec3_t position, struct item *item)
{
  if(item->id != ITEM_NONE)
  {
    const fvec3_t spawn_position = position;
    const fvec3_t spawn_roation = fvec3_zero();
    const fvec3_t spawn_velocity = random_velocity();

    struct entity item_entity;
    entity_init(&item_entity, spawn_position, spawn_roation, spawn_velocity, INFINITY, INFINITY);
    item_entity_init(&item_entity, *item);
    world_add_entity(item_entity);

    item->id = ITEM_NONE;
    item->count = 0;
  }
}

void player_entity_update_health_ui(struct entity *entity, struct player_ui_layout ui_layout)
{
  struct player_opaque *opaque = entity->opaque;

  const unsigned i_max_health = ceilf(entity->max_health);
  const unsigned i_health = ceilf(MAX(entity->health, 0.0f));

  const struct gl_texture_2d heart_full_texture = get_heart_full_texture();
  const struct gl_texture_2d heart_empty_texture = get_heart_empty_texture();
  for(unsigned i=0; i<i_max_health; ++i)
  {
    const struct gl_texture_2d texture = i < i_health ? heart_full_texture : heart_empty_texture;
    const struct ui_rect rect = ui_grid_get_rect_at(ui_layout.health_bar, i, 0);
    ui_rect_textured(rect.position, rect.dimension, rect.rounding, texture.id);
  }

  if(entity->health <= 0.0f)
  {
    window_show_cursor(true);

    // Spill all items on the floor
    {
      // Hand
      spill_item(entity->position, &opaque->hand);

      // Hot bar
      for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
        spill_item(entity->position, &opaque->hotbar[i]);

      // Inventory
      for(unsigned y=0; y<PLAYER_INVENTORY_SIZE_VERTICAL; ++y)
        for(unsigned x=0; x<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++x)
          spill_item(entity->position, &opaque->inventory[y][x]);

      // Crafting inputs
      for(unsigned y=0; y<3; ++y)
        for(unsigned x=0; x<3; ++x)
          spill_item(entity->position, &opaque->crafting_inputs[y][x]);
    }

    // Again I am laying out UI component manually. Sue me.
    const unsigned int height = 100;
    const float margin = 5.0f;

    const char *text = "Respawn";
    const float width = ui_text_width(height, text);

    const fvec2_t position = fvec2(((float)window_size.x - width) * 0.5f, ((float)window_size.y + margin) * 0.5f - (float)height);
    const fvec2_t dimension = fvec2(width, height);

    enum ui_button_result result = ui_button(position, dimension);
    if(result & UI_BUTTON_RESULT_HOVERED)
      ui_quad_colored(position, dimension, 0.1f, fvec4(0.3f, 0.3f, 0.3f, 1.0f));

    ui_text(position, height, 1, text);
    if(result & UI_BUTTON_RESULT_CLICK_LEFT)
    {
      entity_init(entity, opaque->spawn_position, fvec3_zero(), fvec3_zero(), 10.0f, 10.0f);
      window_show_cursor(false);
    }
  }
}
