#include "inventory.h"
#include "../../entity/item/item.h"

#include <voxy/scene/main_game/render/assets.h>
#include <voxy/scene/main_game/states/chunks.h>

#include <libcommon/core/window.h>
#include <libcommon/graphics/gl.h>
#include <libcommon/graphics/ui.h>
#include <libcommon/math/aabb.h>

static inline float minf(float a, float b) { return a < b ? a : b; }

struct cell_widget
{
  fvec2_t position;
  fvec2_t dimension;
  float   rounding;

  fvec4_t default_color;
  fvec4_t highlight_color;

  struct item *item;
  struct item *hand;
};

static void cell_widget_render_background(const struct cell_widget *cell)
{
  fvec4_t color = ui_button(cell->position, cell->dimension) & UI_BUTTON_RESULT_HOVERED ? cell->highlight_color : cell->default_color;
  ui_quad_colored(cell->position, cell->dimension, cell->rounding, color);
}

static void cell_widget_render_item(const struct cell_widget *cell)
{
  if(cell->item->id == ITEM_NONE)
    return;

  struct gl_texture_2d texture = assets_get_item_texture(cell->item->id);
  ui_rect_textured(cell->position, cell->dimension, 0.0f, texture.id);

  char buffer[4];
  snprintf(buffer, sizeof buffer, "%u", cell->item->count);
  ui_text(cell->position, PLAYER_UI_TEXT_SIZE_ITEM_COUNT, 1, buffer);
}

static void cell_widget_update(const struct cell_widget *cell)
{
  int result = ui_button(cell->position, cell->dimension);

  if(result & UI_BUTTON_RESULT_CLICK_LEFT)
  {
    if(cell->hand->id == cell->item->id)
    {
      // Try to pick up all and combine
      unsigned count    = cell->hand->count;
      unsigned capacity = ITEM_MAX_STACK - cell->item->count;
      if(count > capacity)
        count = capacity;

      cell->hand->count -= count;
      cell->item->count += count;
    }
    else
    {
      // Swap
      struct item tmp = *cell->hand;
      *cell->hand = *cell->item;
      *cell->item = tmp;
    }
  }

  if(result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(cell->hand->id == cell->item->id)
    {
      // Try to pick up half and combine
      unsigned count    = cell->item->count / 2;
      unsigned capacity = ITEM_MAX_STACK - cell->hand->count;
      if(count > capacity)
        count = capacity;

      cell->item->count -= count;
      cell->hand->count += count;
    }
    else if(cell->hand->id == ITEM_NONE)
    {
      // Pick up half
      cell->hand->id    = cell->item->id;
      cell->hand->count = cell->item->count / 2;
      cell->item->count -= cell->hand->count;
    }
  }

  if(cell->hand->count == 0) cell->hand->id = ITEM_NONE;
  if(cell->item->count == 0) cell->item->id = ITEM_NONE;
}

struct label_widget
{
  fvec2_t     position;
  unsigned    height;
  const char *text;
};

static void label_widget_render(const struct label_widget *label)
{
  float   width    = ui_text_width(label->height, label->text);
  fvec2_t position = label->position;
  position.x -= width * 0.5f;
  ui_text(position, label->height, 1, label->text);
}


void player_entity_update_inventory(struct entity *entity)
{
  struct player_opaque *opaque = entity->opaque;

  // Inventory UI
  {
    // FIXME: We need a better solution to layout UI than doing it manually.

    /**************************************************
     * Laying out UI elements manually => How Lovely? *
     **************************************************/
    const float base       = minf(window_size.x, window_size.y);
    const float margin     = base * 0.008f;
    const float cell_width = base * 0.05f;
    const float cell_sep   = base * 0.006f;
    const float rounding   = base * 0.006f;

    const float   hotbar_width     = cell_sep + PLAYER_HOTBAR_SIZE * (cell_sep + cell_width);
    const float   hotbar_height    = 2 * cell_sep + cell_width;
    const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);
    const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

    const float   inventory_width     = cell_sep + PLAYER_INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
    const float   inventory_height    = cell_sep + PLAYER_INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
    const fvec2_t inventory_position  = fvec2((window_size.x - inventory_width) * 0.5f, (window_size.y - inventory_height) * 0.5f);
    const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

    const fvec2_t cell_dimension = fvec2(cell_width, cell_width);

    const fvec2_t selected_item_label_position = fvec2(window_size.x * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);
    const fvec2_t target_block_label_position  = fvec2(window_size.x * 0.5f, window_size.y - margin - PLAYER_UI_TEXT_SIZE);

    /********************************************
     * Some basic input handling for UI element *
     ********************************************/
    if(input_press(KEY_I))
    {
      opaque->inventory_opened = !opaque->inventory_opened;
      window_show_cursor(opaque->inventory_opened);
    }

    if(input_press(KEY_1)) opaque->hotbar_selection = 0;
    if(input_press(KEY_2)) opaque->hotbar_selection = 1;
    if(input_press(KEY_3)) opaque->hotbar_selection = 2;
    if(input_press(KEY_4)) opaque->hotbar_selection = 3;
    if(input_press(KEY_5)) opaque->hotbar_selection = 4;
    if(input_press(KEY_6)) opaque->hotbar_selection = 5;
    if(input_press(KEY_7)) opaque->hotbar_selection = 6;
    if(input_press(KEY_8)) opaque->hotbar_selection = 7;
    if(input_press(KEY_9)) opaque->hotbar_selection = 8;

    opaque->hotbar_selection += mouse_scroll.x;
    opaque->hotbar_selection += 9;
    opaque->hotbar_selection %= 9;

    /*********************
     * Major UI Handling *
     *********************/
    struct cell_widget cell_widget;
    cell_widget.dimension = cell_dimension;
    cell_widget.rounding  = rounding;
    cell_widget.hand      = &opaque->hand;

    ui_quad_colored(hotbar_position, hotbar_dimension, rounding, PLAYER_HOTBAR_UI_COLOR_BACKGROUND);
    for(int i=0; i<PLAYER_HOTBAR_SIZE; ++i)
    {
      cell_widget.position        = fvec2_add(fvec2_add(hotbar_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, 0), cell_sep + cell_width));
      cell_widget.default_color   = opaque->hotbar_selection == i ? PLAYER_HOTBAR_UI_COLOR_SELECTED : PLAYER_HOTBAR_UI_COLOR_DEFAULT;
      cell_widget.highlight_color = PLAYER_HOTBAR_UI_COLOR_HOVER;
      cell_widget.item            = &opaque->hotbar[i];

      cell_widget_render_background(&cell_widget);
      cell_widget_render_item(&cell_widget);
      if(opaque->inventory_opened)
        cell_widget_update(&cell_widget);
    }

    if(opaque->inventory_opened)
    {
      ui_quad_colored(inventory_position, inventory_dimension, rounding, PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(int j=0; j<PLAYER_INVENTORY_SIZE_VERTICAL; ++j)
        for(int i=0; i<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++i)
        {
          cell_widget.position        = fvec2_add(fvec2_add(inventory_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, j), cell_sep + cell_width));
          cell_widget.default_color   = PLAYER_INVENTORY_UI_COLOR_DEFAULT;
          cell_widget.highlight_color = PLAYER_INVENTORY_UI_COLOR_HOVER;
          cell_widget.item            = &opaque->inventory[j][i];

          cell_widget_render_background(&cell_widget);
          cell_widget_render_item(&cell_widget);
          cell_widget_update(&cell_widget);
        }
    }

    if(opaque->inventory_opened)
    {
      cell_widget.position = fvec2_sub(mouse_position, fvec2_mul_scalar(fvec2(cell_width, cell_width), 0.5f));
      cell_widget.item     = &opaque->hand;

      cell_widget_render_item(&cell_widget);
    }

    if(!opaque->inventory_opened)
    {
      static bool                 cursor_loaded;
      static struct gl_texture_2d cursor;
      if(!cursor_loaded)
      {
        gl_texture_2d_load(&cursor, "mod/assets/textures/cursor.png");
        cursor_loaded = true;
      }

      fvec2_t dimension = fvec2(32.0f, 32.0f);
      fvec2_t position  = fvec2_mul_scalar(fvec2_sub(ivec2_as_fvec2(window_size), dimension), 0.5f);
      ui_rect_textured(position, dimension, 0.0f, cursor.id);
    }

    if(!opaque->inventory_opened)
    {
      struct label_widget label_widget;
      label_widget.height = PLAYER_UI_TEXT_SIZE;

      const struct item *selected_item = &opaque->hotbar[opaque->hotbar_selection];
      if(selected_item->id != ITEM_NONE)
      {
        label_widget.text     = query_item_info(selected_item->id)->name;
        label_widget.position = selected_item_label_position;
        label_widget_render(&label_widget);
      }

      ivec3_t position;
      ivec3_t normal;
      if(entity_ray_cast(entity, 20.0f, &position, &normal))
      {
        const block_id_t target_block_id = world_get_block_id(position);
        if(target_block_id != BLOCK_NONE && target_block_id != BLOCK_NONE)
        {
          label_widget.text = query_block_info(target_block_id)->name;
          label_widget.position = target_block_label_position;
          label_widget_render(&label_widget);
        }
      }
    }
  }

  // Item Pickup
  {
    const struct entity_info *player_entity_info = query_entity_info(player_entity_id_get());
    const struct entity_info *item_entity_info = query_entity_info(item_entity_id_get());

    aabb3_t hitbox;
    hitbox.center = fvec3_add(entity->position, fvec3_sub(player_entity_info->hitbox_offset, item_entity_info->hitbox_offset));
    hitbox.dimension = fvec3_sub(player_entity_info->hitbox_dimension, item_entity_info->hitbox_dimension);

    const ivec3_t chunk_position_min = get_chunk_position_f(aabb3_min_corner(hitbox));
    const ivec3_t chunk_position_max = get_chunk_position_f(aabb3_max_corner(hitbox));
    for(int z=chunk_position_min.z; z<=chunk_position_max.z; ++z)
      for(int y=chunk_position_min.y; y<=chunk_position_max.y; ++y)
        for(int x=chunk_position_min.x; x<=chunk_position_max.x; ++x)
        {
          const ivec3_t chunk_position = ivec3(x, y, z);
          struct chunk *chunk = world_get_chunk(chunk_position);
          if(!chunk)
            continue;

          for(size_t i=0; i<chunk->entities.item_count; ++i)
          {
            struct entity *other_entity = &chunk->entities.items[i];
            if(entity == other_entity)
              continue;

            if(!entity_intersect(entity, other_entity))
              continue;

            if(other_entity->id != item_entity_id_get())
              continue;

            struct item_opaque *other_opaque = other_entity->opaque;

            for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
              item_merge(&opaque->hotbar[i], &other_opaque->item);

            for(unsigned j=0; j<PLAYER_INVENTORY_SIZE_VERTICAL; ++j)
              for(unsigned i=0; i<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++i)
                item_merge(&opaque->inventory[j][i], &other_opaque->item);

            if(other_opaque->item.id == ITEM_NONE)
            {
              // FIXME: May be we should not be modifying the entities array
              //        during iteration.
              item_entity_fini(other_entity);
              *other_entity = chunk->entities.items[--chunk->entities.item_count];
            }
          }
        }
  }
}
