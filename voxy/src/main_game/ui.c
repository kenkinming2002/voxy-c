#include <voxy/main_game/ui.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/mod_assets.h>

#include <voxy/types/block.h>
#include <voxy/types/player.h>

#include <voxy/graphics/gl.h>
#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>

#include <voxy/config.h>

#include <stdio.h>

static void ui_item_render(struct item *item, fvec2_t position, fvec2_t dimension)
{
  if(item->id != ITEM_NONE)
  {
    struct gl_texture_2d *texture = mod_assets_item_texture_get(item->id);
    ui_quad_textured(position, dimension, 0.0f, texture->id);

    char buffer[4];
    snprintf(buffer, sizeof buffer, "%u", item->count);
    ui_text(position, UI_TEXT_SIZE_ITEM_COUNT, buffer);
  }
}

static void ui_item(struct item *hand, struct item *item, fvec2_t position, fvec2_t dimension, float round, fvec4_t default_color, fvec4_t highlight_color)
{
  int     result = ui_button(position, dimension);
  fvec4_t color  = result & UI_BUTTON_RESULT_HOVERED ? highlight_color : default_color;
  ui_quad_colored(position, dimension, round, color);
  ui_item_render(item, position, dimension);

  if(result & UI_BUTTON_RESULT_CLICK_LEFT)
  {
    if(hand->id == item->id)
    {
      // Try to pick up all and combine
      unsigned count    = hand->count;
      unsigned capacity = ITEM_MAX_STACK - item->count;
      if(count > capacity)
        count = capacity;

      hand->count -= count;
      item->count += count;
    }
    else
    {
      // Swap
      struct item tmp = *hand;
      *hand = *item;
      *item = tmp;
    }
  }

  if(result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(hand->id == item->id)
    {
      // Try to pick up half and combine
      unsigned count    = item->count / 2;
      unsigned capacity = ITEM_MAX_STACK - hand->count;
      if(count > capacity)
        count = capacity;

      item->count -= count;
      hand->count += count;
    }
    else if(hand->id == ITEM_NONE)
    {
      // Pick up half
      hand->id    = item->id;
      hand->count = item->count / 2;
      item->count -= hand->count;
    }
  }
}

/// To any future reader:
///
/// Please do not ever look at the following code plz. It has since become less
/// horrible, but it is still horrible. This is what you get for trying layout
/// UI elements manually in code, which is easier than trying roll a all-rouned
/// UI system I guess!?
static inline float minf(float a, float b)
{
  return a < b ? a : b;
}

void main_game_update_ui()
{
  if(!player_spawned)
    return;

  ////////////////////////////
  /// Open/Close Inventory ///
  ////////////////////////////
  if(input_press(KEY_I))
  {
    player.inventory.opened = !player.inventory.opened;
    window_show_cursor(player.inventory.opened);
  }

  ///////////////////
  ///  UI Metrics ///
  ///////////////////
  const float base       = minf(window_size.x, window_size.y);
  const float margin     = base * 0.008f;
  const float cell_width = base * 0.05f;
  const float cell_sep   = base * 0.006f;
  const float rounding      = base * 0.006f;

  //////////////
  /// Hotbar ///
  //////////////
  const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
  const float   hotbar_height    = 2 * cell_sep + cell_width;
  const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);
  const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

  ui_quad_colored(hotbar_position, hotbar_dimension, rounding, UI_HOTBAR_COLOR_BACKGROUND);
  for(int i=0; i<HOTBAR_SIZE; ++i)
  {
    fvec2_t position  = fvec2_add(hotbar_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep));
    fvec2_t dimension = fvec2(cell_width, cell_width);
    if(i == player.hotbar.selection)
      ui_item(&player.item_held, &player.hotbar.items[i], position, dimension, rounding, UI_HOTBAR_COLOR_SELECTED, UI_HOTBAR_COLOR_HOVER);
    else
      ui_item(&player.item_held, &player.hotbar.items[i], position, dimension, rounding, UI_HOTBAR_COLOR_DEFAULT, UI_HOTBAR_COLOR_HOVER);
  }

  if(input_press(KEY_1)) player.hotbar.selection = 0;
  if(input_press(KEY_2)) player.hotbar.selection = 1;
  if(input_press(KEY_3)) player.hotbar.selection = 2;
  if(input_press(KEY_4)) player.hotbar.selection = 3;
  if(input_press(KEY_5)) player.hotbar.selection = 4;
  if(input_press(KEY_6)) player.hotbar.selection = 5;
  if(input_press(KEY_7)) player.hotbar.selection = 6;
  if(input_press(KEY_8)) player.hotbar.selection = 7;
  if(input_press(KEY_9)) player.hotbar.selection = 8;

  player.hotbar.selection += mouse_scroll.x;
  player.hotbar.selection += 9;
  player.hotbar.selection %= 9;

  /////////////////////
  /// Selected Item ///
  /////////////////////
  const struct item *selected_item = &player.hotbar.items[player.hotbar.selection];
  if(selected_item->id != ITEM_NONE)
  {
    const char *name     = mod_item_info_get(selected_item->id)->name;
    float       width    = ui_text_width(UI_TEXT_SIZE, name);
    fvec2_t     position = fvec2(window_size.x * 0.5f - width * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);
    ui_text(position, UI_TEXT_SIZE, name);
  }

  /////////////////
  /// Inventory ///
  /////////////////
  const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
  const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
  const fvec2_t inventory_position  = fvec2((window_size.x - inventory_width) * 0.5f, (window_size.y - inventory_height) * 0.5f);
  const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);
  if(player.inventory.opened)
  {
    ui_quad_colored(inventory_position, inventory_dimension, rounding, UI_INVENTORY_COLOR_BACKGROUND);
    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        fvec2_t position  = fvec2_add(inventory_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep + j * (cell_sep + cell_width)));
        fvec2_t dimension = fvec2(cell_width, cell_width);
        ui_item(&player.item_held, &player.inventory.items[j][i], position, dimension, rounding, UI_INVENTORY_COLOR_DEFAULT, UI_INVENTORY_COLOR_HOVER);
      }
  }

  ////////////////////
  /// Target Block ///
  ////////////////////
  {
    ivec3_t position;
    ivec3_t normal;
    if(entity_ray_cast(&player.base, 20.0f, &position, &normal))
    {
      const struct block *target_block = world_block_get(position);
      if(target_block && target_block->id != BLOCK_NONE)
      {
        const char   *name     = mod_block_info_get(target_block->id)->name;
        float         width    = ui_text_width(UI_TEXT_SIZE, name);
        const fvec2_t position = fvec2(window_size.x * 0.5f - width * 0.5f, window_size.y - margin - UI_TEXT_SIZE);
        ui_text(position, UI_TEXT_SIZE, name);
      }
    }
  }

  ////////////
  /// Hold ///
  ////////////
  {
    fvec2_t position  = fvec2_sub(mouse_position, fvec2_mul_scalar(fvec2(cell_width, cell_width), 0.5f));
    fvec2_t dimension = fvec2(cell_width, cell_width);
    ui_item_render(&player.item_held, position, dimension);
  }
}

void main_game_render_ui() {}

