#include "inventory.h"
#include "config.h"
#include "entity/item/item.h"

#include <voxy/scene/main_game/render/assets.h>
#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/entity_query.h>
#include <voxy/scene/main_game/crafting/crafting.h>

#include <libcommon/core/window.h>
#include <libcommon/graphics/gl.h>
#include <libcommon/ui/ui.h>
#include <libcommon/math/aabb.h>

static inline void update_slot(struct ui_rect rect, struct item *item, struct item *hand)
{
  const int result = ui_button(rect.position, rect.dimension);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT || result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(hand->id == ITEM_NONE && item->id == ITEM_NONE)
    {
      // Whaya doing!?
    }
    else if(hand->id == ITEM_NONE && item->id != ITEM_NONE)
    {
      const int count = result & UI_BUTTON_RESULT_CLICK_LEFT ? item->count : (item->count + 1) / 2;
      hand->id = item->id;
      hand->count += count;
      item->count -= count;
    }
    else if(hand->id != ITEM_NONE && item->id == ITEM_NONE)
    {
      const int count = result & UI_BUTTON_RESULT_CLICK_LEFT ? hand->count : (hand->count + 1) / 2;
      item->id = hand->id;
      item->count += count;
      hand->count -= count;
    }
    else if(hand->id == item->id)
    {
      const int capacity = ITEM_MAX_STACK - hand->count;
      const int desired_count = result & UI_BUTTON_RESULT_CLICK_LEFT ? item->count : item->count / 2;
      const int count = desired_count < capacity ? desired_count : capacity;
      hand->id = item->id;
      hand->count += count;
      item->count -= count;
    }
    else
    {
      const struct item tmp = *hand;
      *hand = *item;
      *item = tmp;
    }
  }

  if(hand->count == 0)
    hand->id = ITEM_NONE;

  if(item->count == 0)
    item->id = ITEM_NONE;
}

static inline bool update_slot_retrive_only(struct ui_rect rect, struct item item, struct item *hand)
{
  const int result = ui_button(rect.position, rect.dimension);
  if(result & UI_BUTTON_RESULT_CLICK_LEFT || result & UI_BUTTON_RESULT_CLICK_RIGHT)
  {
    if(hand->id == ITEM_NONE)
    {
      *hand = item;
      return true;
    }
    else if(hand->id == item.id && (unsigned)hand->count + (unsigned)item.count <= ITEM_MAX_STACK)
    {
      hand->count += item.count;
      return true;
    }
  }
  return false;
}

static inline void render_background(struct ui_rect rect, fvec4_t color)
{
  ui_quad_colored(rect.position, rect.dimension, rect.rounding, color);
}

static inline void render_item(struct ui_rect rect, struct item item)
{
  if(item.id != ITEM_NONE)
  {
    char buffer[4];
    snprintf(buffer, sizeof buffer, "%u", item.count);

    ui_rect_textured(rect.position, rect.dimension, rect.rounding, assets_get_item_texture(item.id).id);
    ui_text(rect.position, PLAYER_UI_TEXT_SIZE_ITEM_COUNT, 1, buffer);
  }
}

static inline void render_slot(struct ui_rect rect, fvec4_t default_color, fvec4_t hover_color, fvec4_t selected_color, struct item item, bool selected)
{
  const fvec4_t color = ui_button(rect.position, rect.dimension) & UI_BUTTON_RESULT_HOVERED ? hover_color
                      : selected ? selected_color
                      : default_color;

  ui_quad_colored(rect.position, rect.dimension, rect.rounding, color);
  render_item(rect, item);
}

static inline struct gl_texture_2d get_cursor_texture(void)
{
  static bool initialized;
  static struct gl_texture_2d texture;
  if(!initialized)
  {
    gl_texture_2d_load(&texture, "mod/assets/textures/cursor.png");
    initialized = true;
  }
  return texture;
}

void player_entity_update_inventory(struct entity *entity, float dt, struct player_ui_layout ui_layout)
{
  struct player_opaque *opaque = entity->opaque;

  // Force close the inventory to prevent any interaction if we are dead.
  // FIXME: Is this a hack?
  if(entity->health <= 0.0f)
    opaque->inventory_opened = false;

  if(entity->health > 0.0f)
  {
    // Open/close inventory.
    if(input_press(KEY_I))
      opaque->inventory_opened = !opaque->inventory_opened;

    // Select items on hot bar via number keys.
    if(input_press(KEY_1)) opaque->hotbar_selection = 0;
    if(input_press(KEY_2)) opaque->hotbar_selection = 1;
    if(input_press(KEY_3)) opaque->hotbar_selection = 2;
    if(input_press(KEY_4)) opaque->hotbar_selection = 3;
    if(input_press(KEY_5)) opaque->hotbar_selection = 4;
    if(input_press(KEY_6)) opaque->hotbar_selection = 5;
    if(input_press(KEY_7)) opaque->hotbar_selection = 6;
    if(input_press(KEY_8)) opaque->hotbar_selection = 7;
    if(input_press(KEY_9)) opaque->hotbar_selection = 8;

    // Select items on hot bar via scroll wheel.
    opaque->hotbar_selection += mouse_scroll.x;
    opaque->hotbar_selection += 9;
    opaque->hotbar_selection %= 9;
  }

  // Cursor is shown if either inventory is opened or we are dead.
  // FIXME: Is this a hack?
  window_show_cursor(opaque->inventory_opened || entity->health <= 0.0f);

  // Items movement.
  {
    if(opaque->inventory_opened)
    {
      // Hot bar
      for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
        update_slot(ui_grid_get_rect_at(ui_layout.hot_bar, i, 0), &opaque->hotbar[i], &opaque->hand);

      // Inventory
      for(unsigned y=0; y<PLAYER_INVENTORY_SIZE_VERTICAL; ++y)
        for(unsigned x=0; x<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++x)
          update_slot(ui_grid_get_rect_at(ui_layout.inventory, x, y), &opaque->inventory[y][x], &opaque->hand);

      // Crafting inputs
      for(unsigned y=0; y<3; ++y)
        for(unsigned x=0; x<3; ++x)
          update_slot(ui_grid_get_rect_at(ui_layout.crafting_inputs, x, y), &opaque->crafting_inputs[y][x], &opaque->hand);
    }
  }

  // Update crafting system. While it is possible to store crafting_output
  // inside opaque struct, it is not necessary since we recompute it every frame
  // anyway.
  struct item crafting_output;
  {
    const struct recipe *recipe = crafting_find_recipe(opaque->crafting_inputs);
    if(recipe)
    {
      crafting_output = recipe->output;
      if(update_slot_retrive_only(ui_grid_get_rect_at(ui_layout.crafting_output, 0, 0), crafting_output, &opaque->hand))
        recipe_apply(recipe, opaque->crafting_inputs);
    }
    else
    {
      crafting_output.id = ITEM_NONE;
      crafting_output.count = 0;
    }
  }

  // Rendering.
  {
    // Hot bar
    render_background(ui_grid_get_rect_total(ui_layout.hot_bar), PLAYER_HOTBAR_UI_COLOR_BACKGROUND);
    for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
      render_slot(ui_grid_get_rect_at(ui_layout.hot_bar, i, 0), PLAYER_HOTBAR_UI_COLOR_DEFAULT, PLAYER_HOTBAR_UI_COLOR_HOVER, PLAYER_HOTBAR_UI_COLOR_SELECTED, opaque->hotbar[i], i == opaque->hotbar_selection);

    if(opaque->inventory_opened)
    {
      // Inventory
      render_background(ui_grid_get_rect_total(ui_layout.inventory), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(unsigned y=0; y<PLAYER_INVENTORY_SIZE_VERTICAL; ++y)
        for(unsigned x=0; x<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++x)
          render_slot(ui_grid_get_rect_at(ui_layout.inventory, x, y), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), opaque->inventory[y][x], false);

      // Crafting inputs
      render_background(ui_grid_get_rect_total(ui_layout.crafting_inputs), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(unsigned y=0; y<3; ++y)
        for(unsigned x=0; x<3; ++x)
          render_slot(ui_grid_get_rect_at(ui_layout.crafting_inputs, x, y), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), opaque->crafting_inputs[y][x], false);

      // Crafting output
      render_background(ui_grid_get_rect_total(ui_layout.crafting_output), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      render_slot(ui_grid_get_rect_at(ui_layout.crafting_output, 0, 0), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), crafting_output, false);

      // Hand
      render_item(ui_layout.hand, opaque->hand);
    }

    if(!opaque->inventory_opened)
    {
      // Cursor
      {
        const struct gl_texture_2d texture = get_cursor_texture();
        ui_rect_textured(ui_layout.cursor.position, ui_layout.cursor.dimension, ui_layout.cursor.rounding, texture.id);
      }

      // Selected item
      {
        const struct item *selected_item = &opaque->hotbar[opaque->hotbar_selection];
        if(selected_item->id != ITEM_NONE)
        {
          const char *text = query_item_info(selected_item->id)->name;
          ui_text_centered(ui_layout.selected_item_y, ui_layout.selected_item_height, 1, text);
        }
      }

      // Target block
      {
        ivec3_t position;
        ivec3_t normal;
        if(entity_ray_cast(entity, 20.0f, &position, &normal))
        {
          const block_id_t target_block_id = world_get_block_id(position);
          if(target_block_id != BLOCK_NONE)
          {
            const char *text = query_block_info(target_block_id)->name;
            ui_text_centered(ui_layout.target_block_y, ui_layout.target_block_height, 1, text);
          }
        }
      }
    }
  }

  if(entity->health > 0.0f)
  {
    // Item Attraction
    {
      aabb3_t hitbox = entity_hitbox(entity);
      hitbox.dimension = fvec3_add_scalar(hitbox.dimension, PLAYER_ITEM_ATTRACTION_RADIUS);

      struct entity **entities;
      size_t entity_count;
      world_query_entity(hitbox, &entities, &entity_count);

      for(size_t i=0; i<entity_count; ++i)
        if(entities[i]->id == item_entity_id_get())
        {
          const fvec3_t displacement = fvec3_sub(entity->position, entities[i]->position);
          const float distance = fvec3_length(displacement);

          const float attenuation = (1.0f + distance) * (1.0f + distance) * (1.0f + distance);
          const fvec3_t impulse = fvec3_mul_scalar(displacement, PLAYER_ITEM_ATTRACTION_STRENGTH * dt / attenuation);
          entity_apply_impulse(entities[i], impulse);
        }

      free(entities);
    }

    // Item pickup
    {
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
  }
}
