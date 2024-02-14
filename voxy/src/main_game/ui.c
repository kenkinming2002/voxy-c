#include <voxy/main_game/ui.h>
#include <voxy/main_game/world.h>
#include <voxy/main_game/chunks.h>
#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/mod_assets.h>

#include <voxy/types/world.h>
#include <voxy/types/block.h>
#include <voxy/types/player.h>

#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>

#include <voxy/config.h>

#include <stdio.h>

/// To any future reader:
///
/// The following code may look messy and full of code duplication. However, the
/// author has already tried but failed to find any way of refactoring the code
/// without making it significantly more complex. As the saying goes, if it
/// ain't broke, don't fix it.

static inline float minf(float a, float b)
{
  return a < b ? a : b;
}

void main_game_update_ui()
{
  if(!player_spawned)
    return;

  ///////////////////
  ///  UI Metrics ///
  ///////////////////
  const float base       = minf(window_size.x, window_size.y);
  const float margin     = base * 0.008f;
  const float cell_width = base * 0.05f;
  const float cell_sep   = base * 0.006f;

  ////////////////////////////
  /// Open/Close Inventory ///
  ////////////////////////////
  if(input_press(KEY_I))
  {
    player.inventory.opened = !player.inventory.opened;
    window_show_cursor(player.inventory.opened);
  }

  if(player.inventory.opened)
  {
    player.item_hovered = NULL;

    ///////////////
    ///  Hotbar ///
    ///////////////
    {
      const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
      const float   hotbar_height    = 2 * cell_sep + cell_width;
      const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);
      const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

      (void)hotbar_dimension;

      const int i = floorf((mouse_position.x - hotbar_position.x - cell_sep) / (cell_sep + cell_width));
      const int j = floorf((mouse_position.y - hotbar_position.y - cell_sep) / (cell_sep + cell_width));
      if(0 <= i && i < HOTBAR_SIZE && j == 0)
        player.item_hovered = &player.hotbar.items[i];
    }

    /////////////////
    /// Inventory ///
    /////////////////
    {
      const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
      const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
      const fvec2_t inventory_position  = fvec2((window_size.x - inventory_width) * 0.5f, (window_size.y - inventory_height) * 0.5f);
      const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

      (void)inventory_dimension;

      const int i = floorf((mouse_position.x - inventory_position.x - cell_sep) / (cell_sep + cell_width));
      const int j = floorf((mouse_position.y - inventory_position.y - cell_sep) / (cell_sep + cell_width));
      if(0 <= i && i < INVENTORY_SIZE_HORIZONTAL && 0 <= j && j < INVENTORY_SIZE_VERTICAL)
        player.item_hovered = &player.inventory.items[j][i];
    }

    /////////////////////
    /// Item Movement ///
    /////////////////////
    {
      if(player.item_hovered)
      {
        if(input_press(BUTTON_LEFT))
        {
          if(player.item_held.id == player.item_hovered->id)
          {
            unsigned count    = player.item_held.count;
            unsigned capacity = ITEM_MAX_STACK - player.item_hovered->count;
            if(count > capacity)
              count = capacity;

            player.item_held.count     -= count;
            player.item_hovered->count += count;
          }
          else
          {
            struct item tmp = player.item_held;
            player.item_held = *player.item_hovered;
            *player.item_hovered = tmp;
          }
        }
        else if(input_press(BUTTON_RIGHT))
        {
          if(player.item_held.id == player.item_hovered->id)
          {
            unsigned count    = player.item_hovered->count / 2;
            unsigned capacity = ITEM_MAX_STACK - player.item_held.count;
            if(count > capacity)
              count = capacity;

            player.item_hovered->count -= count;
            player.item_held.count     += count;
          }
          else if(player.item_held.id == ITEM_NONE)
          {
            player.item_held.id    = player.item_hovered->id;
            player.item_held.count = player.item_hovered->count / 2;
            player.item_hovered->count -= player.item_held.count;
          }
        }

        if(player.item_hovered->count == 0) player.item_hovered->id = ITEM_NONE;
        if(player.item_held.count     == 0) player.item_held.id     = ITEM_NONE;
      }
      player.item_held_position = mouse_position;
    }
  }
  else
  {
    /////////////////////////////
    /// Hotbar item selection ///
    /////////////////////////////
    {
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
    }
  }
}

void main_game_render_ui()
{
  if(!player_spawned)
    return;

  {
    ///////////////
    /// Metrics ///
    ///////////////
    const float base       = minf(window_size.x, window_size.y);
    const float margin     = base * 0.008f;
    const float cell_width = base * 0.05f;
    const float cell_sep   = base * 0.006f;
    const float round      = base * 0.006f;

    //////////////
    /// Hotbar ///
    //////////////
    const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
    const float   hotbar_height    = 2 * cell_sep + cell_width;
    const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);
    const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

    ui_draw_quad_rounded(hotbar_position, hotbar_dimension, round, UI_HOTBAR_COLOR_BACKGROUND);

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      const fvec2_t cell_position  = fvec2_add(hotbar_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep));
      const fvec2_t cell_dimension = fvec2(cell_width, cell_width);
      const fvec4_t cell_color     = &player.hotbar.items[i] == player.item_hovered ? UI_HOTBAR_COLOR_HOVER : i == player.hotbar.selection ? UI_HOTBAR_COLOR_SELECTED : UI_HOTBAR_COLOR_DEFAULT;


      ui_draw_quad_rounded(cell_position, cell_dimension, round, cell_color);

      const struct item *cell_item = &player.hotbar.items[i];
      if(cell_item->id != ITEM_NONE)
      {
        const uint8_t               cell_item_id      = cell_item->id;
        const uint8_t               cell_item_count   = cell_item->count;
        const struct gl_texture_2d *cell_item_texture = mod_assets_item_texture_get(cell_item_id);

        char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
        snprintf(buffer, sizeof buffer, "%u", cell_item_count);

        ui_draw_texture(cell_position, cell_dimension, cell_item_texture->id);
        ui_draw_text(mod_assets_font_set_get(), cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
      }
    }

    /////////////////////
    /// Selected Item ///
    /////////////////////
    const fvec2_t selected_item_text_position = fvec2(window_size.x * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);

    const struct item *selected_item      = &player.hotbar.items[player.hotbar.selection];
    const uint8_t      selected_item_id   = selected_item->id;
    const char        *selected_item_name = selected_item_id != ITEM_NONE ? mod_item_info_get(selected_item_id)->name : NULL;
    if(selected_item_name)
      ui_draw_text_centered(mod_assets_font_set_get(), selected_item_text_position, selected_item_name, UI_TEXT_SIZE);

    /////////////////
    /// Inventory ///
    /////////////////
    const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
    const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
    const fvec2_t inventory_position  = fvec2((window_size.x - inventory_width) * 0.5f, (window_size.y - inventory_height) * 0.5f);
    const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

    if(player.inventory.opened)
    {
      ui_draw_quad_rounded(inventory_position, inventory_dimension, round, UI_INVENTORY_COLOR_BACKGROUND);

      for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
        for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
        {
          const fvec2_t cell_position  = fvec2_add(inventory_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep + j * (cell_sep + cell_width)));
          const fvec2_t cell_dimension = fvec2(cell_width, cell_width);
          const fvec4_t cell_color     = &player.inventory.items[j][i] == player.item_hovered ? UI_INVENTORY_COLOR_HOVER : UI_INVENTORY_COLOR_DEFAULT;

          ui_draw_quad_rounded(cell_position, cell_dimension, round, cell_color);

          const struct item *cell_item = &player.inventory.items[j][i];
          if(cell_item->id != ITEM_NONE)
          {
            const uint8_t               cell_item_id      = cell_item->id;
            const uint8_t               cell_item_count   = cell_item->count;
            const struct gl_texture_2d *cell_item_texture = mod_assets_item_texture_get(cell_item_id);

            char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
            snprintf(buffer, sizeof buffer, "%u", cell_item_count);

            ui_draw_texture(cell_position, cell_dimension, cell_item_texture->id);
            ui_draw_text(mod_assets_font_set_get(), cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
          }
        }
    }

    ///////////////////
    /// Target Block ///
    ///////////////////
    const fvec2_t target_block_text_position = fvec2(window_size.x * 0.5f, window_size.y - margin - UI_TEXT_SIZE);

    ivec3_t position;
    ivec3_t normal;
    if(entity_ray_cast(&player.base, 20.0f, &position, &normal))
    {
      const struct block *target_block      = block_get(position);
      const uint8_t       target_block_id   = target_block ? target_block->id : BLOCK_NONE;
      const char         *target_block_name = target_block_id != BLOCK_NONE ? mod_block_info_get(target_block_id)->name : NULL;
      if(target_block_name)
        ui_draw_text_centered(mod_assets_font_set_get(), target_block_text_position, target_block_name, UI_TEXT_SIZE);
    }


    ////////////
    /// Hold ///
    ////////////
    {
      const fvec2_t cell_position  = fvec2_sub(player.item_held_position, fvec2_mul_scalar(fvec2(cell_width, cell_width), 0.5f));
      const fvec2_t cell_dimension = fvec2(cell_width, cell_width);

      const struct item *cell_item = &player.item_held;
      if(cell_item->id != ITEM_NONE)
      {
        const uint8_t               cell_item_id      = cell_item->id;
        const uint8_t               cell_item_count   = cell_item->count;
        const struct gl_texture_2d *cell_item_texture = mod_assets_item_texture_get(cell_item_id);

        char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
        snprintf(buffer, sizeof buffer, "%u", cell_item_count);

        ui_draw_texture(cell_position, cell_dimension, cell_item_texture->id);
        ui_draw_text(mod_assets_font_set_get(), cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
      }
    }
  }
}
