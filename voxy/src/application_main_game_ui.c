#include "application_main_game.h"

#include "window.h"
#include "renderer_ui.h"

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

void application_main_game_update_ui(struct application_main_game *application_main_game, struct window *window)
{
  if(!application_main_game->world.player.spawned)
    return;

  struct world  *world  = &application_main_game->world;
  struct player *player = &world->player;

  ///////////////////
  ///  UI Metrics ///
  ///////////////////
  const float base       = minf(window->width, window->height);
  const float margin     = base * 0.008f;
  const float cell_width = base * 0.05f;
  const float cell_sep   = base * 0.006f;

  ////////////////////////////
  /// Open/Close Inventory ///
  ////////////////////////////
  if(window->presses & (1ULL << KEY_I))
  {
    world->player.inventory.opened = !world->player.inventory.opened;
    window_set_cursor(window, player->inventory.opened);
  }

  if(player->inventory.opened)
  {
    player->item_hovered = NULL;

    ///////////////
    ///  Hotbar ///
    ///////////////
    {
      const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
      const float   hotbar_height    = 2 * cell_sep + cell_width;
      const fvec2_t hotbar_position  = fvec2((window->width - hotbar_width) * 0.5f, margin);
      const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

      (void)hotbar_dimension;

      const int i = floorf((window->mouse_position.x - hotbar_position.x - cell_sep) / (cell_sep + cell_width));
      const int j = floorf((window->mouse_position.y - hotbar_position.y - cell_sep) / (cell_sep + cell_width));
      if(0 <= i && i < HOTBAR_SIZE && j == 0)
        player->item_hovered = &player->hotbar.items[i];
    }

    /////////////////
    /// Inventory ///
    /////////////////
    {
      const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
      const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
      const fvec2_t inventory_position  = fvec2((window->width - inventory_width) * 0.5f, (window->height - inventory_height) * 0.5f);
      const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

      (void)inventory_dimension;

      const int i = floorf((window->mouse_position.x - inventory_position.x - cell_sep) / (cell_sep + cell_width));
      const int j = floorf((window->mouse_position.y - inventory_position.y - cell_sep) / (cell_sep + cell_width));
      if(0 <= i && i < INVENTORY_SIZE_HORIZONTAL && 0 <= j && j < INVENTORY_SIZE_VERTICAL)
        player->item_hovered = &player->inventory.items[j][i];
    }

    /////////////////////
    /// Item Movement ///
    /////////////////////
    {
      if(player->item_hovered)
      {
        if(window->presses & (1ULL << BUTTON_LEFT))
        {
          if(player->item_held.id == player->item_hovered->id)
          {
            unsigned count    = player->item_held.count;
            unsigned capacity = ITEM_MAX_STACK - player->item_hovered->count;
            if(count > capacity)
              count = capacity;

            player->item_held.count     -= count;
            player->item_hovered->count += count;
          }
          else
          {
            struct item tmp = player->item_held;
            player->item_held = *player->item_hovered;
            *player->item_hovered = tmp;
          }
        }
        else if(window->presses & (1ULL << BUTTON_RIGHT))
        {
          if(player->item_held.id == player->item_hovered->id)
          {
            unsigned count    = player->item_hovered->count / 2;
            unsigned capacity = ITEM_MAX_STACK - player->item_held.count;
            if(count > capacity)
              count = capacity;

            player->item_hovered->count -= count;
            player->item_held.count     += count;
          }
          else if(player->item_held.id == ITEM_NONE)
          {
            player->item_held.id    = player->item_hovered->id;
            player->item_held.count = player->item_hovered->count / 2;
            player->item_hovered->count -= player->item_held.count;
          }
        }

        if(player->item_hovered->count == 0) player->item_hovered->id = ITEM_NONE;
        if(player->item_held.count     == 0) player->item_held.id     = ITEM_NONE;
      }
      player->item_held_position = window->mouse_position;
    }
  }
  else
  {
    /////////////////////////////
    /// Hotbar item selection ///
    /////////////////////////////
    {
      if(window->presses & (1ULL << KEY_1)) world->player.hotbar.selection = 0;
      if(window->presses & (1ULL << KEY_2)) world->player.hotbar.selection = 1;
      if(window->presses & (1ULL << KEY_3)) world->player.hotbar.selection = 2;
      if(window->presses & (1ULL << KEY_4)) world->player.hotbar.selection = 3;
      if(window->presses & (1ULL << KEY_5)) world->player.hotbar.selection = 4;
      if(window->presses & (1ULL << KEY_6)) world->player.hotbar.selection = 5;
      if(window->presses & (1ULL << KEY_7)) world->player.hotbar.selection = 6;
      if(window->presses & (1ULL << KEY_8)) world->player.hotbar.selection = 7;
      if(window->presses & (1ULL << KEY_9)) world->player.hotbar.selection = 8;

      world->player.hotbar.selection += window->mouse_scroll.x;
      world->player.hotbar.selection += 9;
      world->player.hotbar.selection %= 9;
    }
  }
}

void application_main_game_render_ui(struct application_main_game *application_main_game, struct window *window, struct renderer_ui *renderer_ui)
{
  if(!application_main_game->world.player.spawned)
    return;

  struct resource_pack *resource_pack = &application_main_game->resource_pack;
  struct world         *world         = &application_main_game->world;
  struct player        *player        = &world->player;

  renderer_ui_begin(renderer_ui, fvec2(window->width, window->height));
  {
    ///////////////
    /// Metrics ///
    ///////////////
    const float base       = minf(window->width, window->height);
    const float margin     = base * 0.008f;
    const float cell_width = base * 0.05f;
    const float cell_sep   = base * 0.006f;
    const float round      = base * 0.006f;

    //////////////
    /// Hotbar ///
    //////////////
    const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
    const float   hotbar_height    = 2 * cell_sep + cell_width;
    const fvec2_t hotbar_position  = fvec2((window->width - hotbar_width) * 0.5f, margin);
    const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

    renderer_ui_draw_quad_rounded(renderer_ui, hotbar_position, hotbar_dimension, round, UI_HOTBAR_COLOR_BACKGROUND);

    for(int i=0; i<HOTBAR_SIZE; ++i)
    {
      const fvec2_t cell_position  = fvec2_add(hotbar_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep));
      const fvec2_t cell_dimension = fvec2(cell_width, cell_width);
      const fvec4_t cell_color     = &player->hotbar.items[i] == player->item_hovered ? UI_HOTBAR_COLOR_HOVER : i == player->hotbar.selection ? UI_HOTBAR_COLOR_SELECTED : UI_HOTBAR_COLOR_DEFAULT;


      renderer_ui_draw_quad_rounded(renderer_ui, cell_position, cell_dimension, round, cell_color);

      const struct item *cell_item = &player->hotbar.items[i];
      if(cell_item->id != ITEM_NONE)
      {
        const uint8_t               cell_item_id      = cell_item->id;
        const uint8_t               cell_item_count   = cell_item->count;
        const struct gl_texture_2d *cell_item_texture = &resource_pack->item_textures[cell_item_id];

        char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
        snprintf(buffer, sizeof buffer, "%u", cell_item_count);

        renderer_ui_draw_texture(renderer_ui, cell_position, cell_dimension, cell_item_texture->id);
        renderer_ui_draw_text(renderer_ui, &resource_pack->font_set, cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
      }
    }

    /////////////////////
    /// Selected Item ///
    /////////////////////
    const fvec2_t selected_item_text_position = fvec2(window->width * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);

    const struct item *selected_item      = &player->hotbar.items[player->hotbar.selection];
    const uint8_t      selected_item_id   = selected_item->id;
    const char        *selected_item_name = selected_item_id != ITEM_NONE ? application_main_game->resource_pack.item_infos[selected_item_id].name : NULL;
    if(selected_item_name)
      renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, selected_item_text_position, selected_item_name, UI_TEXT_SIZE);

    /////////////////
    /// Inventory ///
    /////////////////
    const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
    const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
    const fvec2_t inventory_position  = fvec2((window->width - inventory_width) * 0.5f, (window->height - inventory_height) * 0.5f);
    const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

    if(player->inventory.opened)
    {
      renderer_ui_draw_quad_rounded(renderer_ui, inventory_position, inventory_dimension, round, UI_INVENTORY_COLOR_BACKGROUND);

      for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
        for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
        {
          const fvec2_t cell_position  = fvec2_add(inventory_position, fvec2(cell_sep + i * (cell_sep + cell_width), cell_sep + j * (cell_sep + cell_width)));
          const fvec2_t cell_dimension = fvec2(cell_width, cell_width);
          const fvec4_t cell_color     = &player->inventory.items[j][i] == player->item_hovered ? UI_INVENTORY_COLOR_HOVER : UI_INVENTORY_COLOR_DEFAULT;

          renderer_ui_draw_quad_rounded(renderer_ui, cell_position, cell_dimension, round, cell_color);

          const struct item *cell_item = &player->inventory.items[j][i];
          if(cell_item->id != ITEM_NONE)
          {
            const uint8_t               cell_item_id      = cell_item->id;
            const uint8_t               cell_item_count   = cell_item->count;
            const struct gl_texture_2d *cell_item_texture = &resource_pack->item_textures[cell_item_id];

            char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
            snprintf(buffer, sizeof buffer, "%u", cell_item_count);

            renderer_ui_draw_texture(renderer_ui, cell_position, cell_dimension, cell_item_texture->id);
            renderer_ui_draw_text(renderer_ui, &resource_pack->font_set, cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
          }
        }
    }

    ///////////////////
    /// Target Tile ///
    ///////////////////
    const fvec2_t target_tile_text_position = fvec2(window->width * 0.5f, window->height - margin - UI_TEXT_SIZE);

    const struct tile *target_tile      = application_main_game->world.player.has_target_destroy ? world_get_tile(&application_main_game->world, application_main_game->world.player.target_destroy) : NULL;
    const uint8_t      target_tile_id   = target_tile ? target_tile->id : BLOCK_NONE;
    const char        *target_tile_name = target_tile_id != BLOCK_NONE ? resource_pack->block_infos[target_tile_id].name : NULL;
    if(target_tile_name)
      renderer_ui_draw_text_centered(renderer_ui, &application_main_game->resource_pack.font_set, target_tile_text_position, target_tile_name, UI_TEXT_SIZE);

    ////////////
    /// Hold ///
    ////////////
    {
      const fvec2_t cell_position  = fvec2_sub(player->item_held_position, fvec2_mul_scalar(fvec2(cell_width, cell_width), 0.5f));
      const fvec2_t cell_dimension = fvec2(cell_width, cell_width);

      const struct item *cell_item = &player->item_held;
      if(cell_item->id != ITEM_NONE)
      {
        const uint8_t               cell_item_id      = cell_item->id;
        const uint8_t               cell_item_count   = cell_item->count;
        const struct gl_texture_2d *cell_item_texture = &resource_pack->item_textures[cell_item_id];

        char buffer[4]; // Max item count is UINT8_MAX == 255. Hence 3 digits is sufficient
        snprintf(buffer, sizeof buffer, "%u", cell_item_count);

        renderer_ui_draw_texture(renderer_ui, cell_position, cell_dimension, cell_item_texture->id);
        renderer_ui_draw_text(renderer_ui, &resource_pack->font_set, cell_position, buffer, UI_TEXT_SIZE_ITEM_COUNT);
      }
    }
  }
}

