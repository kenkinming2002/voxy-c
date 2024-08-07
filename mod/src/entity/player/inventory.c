#include "inventory.h"
#include "../../entity/item/item.h"

#include <voxy/scene/main_game/render/assets.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/entity_query.h>
#include <voxy/scene/main_game/crafting/crafting.h>

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
  const int result = ui_button(cell->position, cell->dimension);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT || result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(cell->hand->id == ITEM_NONE && cell->item->id == ITEM_NONE)
    {
      // Whaya doing!?
    }
    else if(cell->hand->id == ITEM_NONE && cell->item->id != ITEM_NONE)
    {
      const int count = result & UI_BUTTON_RESULT_CLICK_LEFT ? cell->item->count : (cell->item->count + 1) / 2;
      cell->hand->id = cell->item->id;
      cell->hand->count += count;
      cell->item->count -= count;
    }
    else if(cell->hand->id != ITEM_NONE && cell->item->id == ITEM_NONE)
    {
      const int count = result & UI_BUTTON_RESULT_CLICK_LEFT ? cell->hand->count : (cell->hand->count + 1) / 2;
      cell->item->id = cell->hand->id;
      cell->item->count += count;
      cell->hand->count -= count;
    }
    else if(cell->hand->id == cell->item->id)
    {
      const int capacity = ITEM_MAX_STACK - cell->hand->count;
      const int desired_count = result & UI_BUTTON_RESULT_CLICK_LEFT ? cell->item->count : cell->item->count / 2;
      const int count = desired_count < capacity ? desired_count : capacity;
      cell->hand->id = cell->item->id;
      cell->hand->count += count;
      cell->item->count -= count;
    }
    else
    {
      const struct item tmp = *cell->hand;
      *cell->hand = *cell->item;
      *cell->item = tmp;
    }
  }

  if(cell->hand->count == 0)
    cell->hand->id = ITEM_NONE;

  if(cell->item->count == 0)
    cell->item->id = ITEM_NONE;
}

static bool cell_widget_update_retrive(const struct cell_widget *cell)
{
  const int result = ui_button(cell->position, cell->dimension);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT || result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(cell->hand->id == ITEM_NONE)
    {
      *cell->hand = *cell->item;
      return true;
    }
    else if(cell->hand->id == cell->item->id && (unsigned)cell->hand->count + (unsigned)cell->item->count <= ITEM_MAX_STACK)
    {
      cell->hand->count += cell->item->count;
      return true;
    }
  }
  return false;
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

    const float   crafting_inputs_width     = cell_sep + 3 * (cell_sep + cell_width);
    const float   crafting_inputs_height    = cell_sep + 3 * (cell_sep + cell_width);
    const fvec2_t crafting_inputs_position  = fvec2((window_size.x - crafting_inputs_width) * 0.5f - 2 * (cell_sep + cell_width), (window_size.y + inventory_height) * 0.5f + (cell_sep + cell_width));
    const fvec2_t crafting_inputs_dimension = fvec2(crafting_inputs_width, crafting_inputs_height);

    const float   crafting_output_width     = cell_sep + 1 * (cell_sep + cell_width);
    const float   crafting_output_height    = cell_sep + 1 * (cell_sep + cell_width);
    const fvec2_t crafting_output_position  = fvec2((window_size.x - crafting_output_width) * 0.5f + 2 * (cell_sep + cell_width), (window_size.y + inventory_height) * 0.5f + 2 * (cell_sep + cell_width));
    const fvec2_t crafting_output_dimension = fvec2(crafting_output_width, crafting_output_height);

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
      ui_quad_colored(crafting_inputs_position, crafting_inputs_dimension, rounding, PLAYER_CRAFTING_UI_COLOR_BACKGROUND);
      for(int j=0; j<3; ++j)
        for(int i=0; i<3; ++i)
        {
          cell_widget.position        = fvec2_add(fvec2_add(crafting_inputs_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, j), cell_sep + cell_width));
          cell_widget.default_color   = PLAYER_CRAFTING_UI_COLOR_DEFAULT;
          cell_widget.highlight_color = PLAYER_CRAFTING_UI_COLOR_HOVER;
          cell_widget.item            = &opaque->crafting_inputs[j][i];

          cell_widget_render_background(&cell_widget);
          cell_widget_render_item(&cell_widget);
          cell_widget_update(&cell_widget);
        }

    }

    struct item crafting_output;
    crafting_output.id = ITEM_NONE;
    crafting_output.count = 0;

    const struct recipe *recipe = crafting_find_recipe(opaque->crafting_inputs);
    if(recipe)
      crafting_output = recipe->output;

    if(opaque->inventory_opened)
    {
      ui_quad_colored(crafting_output_position, crafting_output_dimension, rounding, PLAYER_CRAFTING_UI_COLOR_BACKGROUND);

      cell_widget.position        = fvec2_add(crafting_output_position, fvec2(cell_sep, cell_sep));
      cell_widget.default_color   = PLAYER_CRAFTING_UI_COLOR_DEFAULT;
      cell_widget.highlight_color = PLAYER_CRAFTING_UI_COLOR_HOVER;
      cell_widget.item            = &crafting_output;

      cell_widget_render_background(&cell_widget);
      cell_widget_render_item(&cell_widget);
      if(recipe && cell_widget_update_retrive(&cell_widget))
        recipe_apply(recipe, opaque->crafting_inputs);
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

  const aabb3_t hitbox = entity_hitbox(entity);

  struct entity **entities;
  size_t entity_count;
  world_query_entity(hitbox, &entities, &entity_count);

  for(size_t i=0; i<entity_count; ++i)
    if(entities[i]->id == item_entity_id_get())
    {
      struct item_opaque *other_opaque = entities[i]->opaque;

      for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
        item_merge(&opaque->hotbar[i], &other_opaque->item);

      for(unsigned j=0; j<PLAYER_INVENTORY_SIZE_VERTICAL; ++j)
        for(unsigned i=0; i<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++i)
          item_merge(&opaque->inventory[j][i], &other_opaque->item);

      if(other_opaque->item.id == ITEM_NONE)
        entities[i]->remove = true;
    }

  free(entities);
}
