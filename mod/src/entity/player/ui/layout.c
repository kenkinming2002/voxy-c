#include "layout.h"
#include "../config.h"
#include "../player.h"

#include <libcommon/core/window.h>
#include <libcommon/utils/utils.h>

static inline float ui_scale(float factor)
{
  return factor * MIN(window_size.x, window_size.y);
}

struct player_ui_layout compute_player_ui_layout(struct entity *entity)
{
  struct player_opaque *opaque = entity->opaque;

  struct player_ui_layout ui_layout = {0};

  const float slot_width = ui_scale(0.05f);
  const float slot_spacing = ui_scale(0.006f);
  const float slot_rounding = ui_scale(0.006f);

  const float heart_width = ui_scale(0.025f);

  ui_layout.hand.position = fvec2_sub(mouse_position, fvec2_mul_scalar(fvec2(slot_width, slot_width), 0.5f));
  ui_layout.hand.dimension = fvec2(slot_width, slot_width);

  ui_layout.hot_bar.width = PLAYER_HOTBAR_SIZE;
  ui_layout.hot_bar.height = 1;
  ui_layout.hot_bar.spacing = fvec2(slot_spacing, slot_spacing);
  ui_layout.hot_bar.dimension = fvec2(slot_width, slot_width);
  ui_layout.hot_bar.rounding = slot_rounding;

  ui_layout.inventory.width = PLAYER_INVENTORY_SIZE_HORIZONTAL;
  ui_layout.inventory.height = PLAYER_INVENTORY_SIZE_VERTICAL;
  ui_layout.inventory.spacing = fvec2(slot_spacing, slot_spacing);
  ui_layout.inventory.dimension = fvec2(slot_width, slot_width);
  ui_layout.inventory.rounding = slot_rounding;

  ui_layout.crafting_inputs.width = 3;
  ui_layout.crafting_inputs.height = 3;
  ui_layout.crafting_inputs.spacing = fvec2(slot_spacing, slot_spacing);
  ui_layout.crafting_inputs.dimension = fvec2(slot_width, slot_width);
  ui_layout.crafting_inputs.rounding = slot_rounding;

  ui_layout.crafting_output.width = 1;
  ui_layout.crafting_output.height = 1;
  ui_layout.crafting_output.spacing = fvec2(slot_spacing, slot_spacing);
  ui_layout.crafting_output.dimension = fvec2(slot_width, slot_width);
  ui_layout.crafting_output.rounding = slot_rounding;

  ui_layout.container.width = opaque->container.width;
  ui_layout.container.height = opaque->container.height;
  ui_layout.container.spacing = fvec2(slot_spacing, slot_spacing);
  ui_layout.container.dimension = fvec2(slot_width, slot_width);
  ui_layout.container.rounding = slot_rounding;

  ui_layout.health_bar.width = -1;
  ui_layout.health_bar.height = 1;
  ui_layout.health_bar.spacing = fvec2_zero();
  ui_layout.health_bar.dimension = fvec2(heart_width, heart_width);
  ui_layout.health_bar.rounding = 0.0f;

  const float margin = ui_scale(0.008f);

  ui_grid_set_position_x(&ui_layout.hot_bar, UI_ANCHOR_CENTER, window_size.x * 0.5f);
  ui_grid_set_position_y(&ui_layout.hot_bar, UI_ANCHOR_LOW, margin);

  ui_grid_set_position_x(&ui_layout.health_bar, UI_ANCHOR_LOW, ui_rect_left(ui_grid_get_rect_total(ui_layout.hot_bar)));
  ui_grid_set_position_y(&ui_layout.health_bar, UI_ANCHOR_LOW, ui_rect_top(ui_grid_get_rect_total(ui_layout.hot_bar)) + margin);

  ui_layout.selected_item_y = ui_rect_top(ui_grid_get_rect_total(ui_layout.health_bar)) + margin;
  ui_layout.selected_item_height = PLAYER_UI_TEXT_SIZE;

  ui_grid_set_position(&ui_layout.inventory, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER, fvec2_mul_scalar(ivec2_as_fvec2(window_size), 0.5f));

  const float crafting_inputs_height = ui_rect_height(ui_grid_get_rect_total(ui_layout.crafting_inputs));
  const float crafting_output_height = ui_rect_height(ui_grid_get_rect_total(ui_layout.crafting_output));
  const float crafting_max_height = MAX(crafting_inputs_height, crafting_output_height);

  ui_grid_set_position_x(&ui_layout.crafting_inputs, UI_ANCHOR_CENTER, ui_rect_left(ui_grid_get_rect_total(ui_layout.inventory)) + ui_rect_width(ui_grid_get_rect_total(ui_layout.inventory)) * 1.0f / 3.0f);
  ui_grid_set_position_x(&ui_layout.crafting_output, UI_ANCHOR_CENTER, ui_rect_left(ui_grid_get_rect_total(ui_layout.inventory)) + ui_rect_width(ui_grid_get_rect_total(ui_layout.inventory)) * 2.0f / 3.0f);

  ui_grid_set_position_y(&ui_layout.crafting_inputs, UI_ANCHOR_CENTER, ui_rect_top(ui_grid_get_rect_total(ui_layout.inventory)) + margin + crafting_max_height * 0.5f);
  ui_grid_set_position_y(&ui_layout.crafting_output, UI_ANCHOR_CENTER, ui_rect_top(ui_grid_get_rect_total(ui_layout.inventory)) + margin + crafting_max_height * 0.5f);

  ui_grid_set_position_x(&ui_layout.container, UI_ANCHOR_CENTER, window_size.x * 0.5f);
  ui_grid_set_position_y(&ui_layout.container, UI_ANCHOR_LOW, ui_rect_top(ui_grid_get_rect_total(ui_layout.inventory)) + margin);

  ui_layout.target_block_y = window_size.y - PLAYER_UI_TEXT_SIZE - margin;
  ui_layout.target_block_height = PLAYER_UI_TEXT_SIZE;

  ui_layout.cursor.dimension = fvec2(32.0f, 32.0f);
  ui_rect_set_position(&ui_layout.cursor, UI_ANCHOR_CENTER, UI_ANCHOR_CENTER, fvec2_mul_scalar(ivec2_as_fvec2(window_size), 0.5f));

  return ui_layout;
}
