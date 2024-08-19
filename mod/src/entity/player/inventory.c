#include "inventory.h"
#include "config.h"
#include "entity/item/item.h"

#include <voxy/scene/main_game/types/container.h>
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

    ui_rect_textured(rect.position, rect.dimension, rect.rounding, assets_get_item_texture(item.id)->id);
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
    opaque->ui_state = PLAYER_UI_STATE_DEFAULT;

  if(entity->health > 0.0f)
  {
    // Open/close inventory.
    if(input_press(KEY_I))
        opaque->ui_state = opaque->ui_state == PLAYER_UI_STATE_DEFAULT ? PLAYER_UI_STATE_INVENTORY_OPENED : PLAYER_UI_STATE_DEFAULT;

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

  if(opaque->container)
  {
    // Check if container is destroyed.
    if(opaque->container->strong_count == 0)
    {
      container_put_weak(&opaque->container);
      if(opaque->ui_state == PLAYER_UI_STATE_CONTAINER_OPENED)
        opaque->ui_state = PLAYER_UI_STATE_DEFAULT;
    }
    // Check if our ui state have changed
    else if(opaque->ui_state != PLAYER_UI_STATE_CONTAINER_OPENED)
      container_put_weak(&opaque->container);
  }

  // Cursor is shown if either inventory is opened or we are dead.
  // FIXME: Is this a hack?
  window_show_cursor(opaque->ui_state != PLAYER_UI_STATE_DEFAULT || entity->health <= 0.0f);

  // Update
  struct item crafting_output;
  {
    if(opaque->ui_state != PLAYER_UI_STATE_DEFAULT)
    {
      // Hot bar
      for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
        update_slot(ui_grid_get_rect_at(ui_layout.hot_bar, i, 0), &opaque->hotbar[i], &opaque->hand);

      // Inventory
      for(unsigned y=0; y<PLAYER_INVENTORY_SIZE_VERTICAL; ++y)
        for(unsigned x=0; x<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++x)
          update_slot(ui_grid_get_rect_at(ui_layout.inventory, x, y), &opaque->inventory[y][x], &opaque->hand);
    }

    if(opaque->ui_state == PLAYER_UI_STATE_INVENTORY_OPENED)
    {
      // Crafting inputs
      for(unsigned y=0; y<3; ++y)
        for(unsigned x=0; x<3; ++x)
          update_slot(ui_grid_get_rect_at(ui_layout.crafting_inputs, x, y), &opaque->crafting_inputs[y][x], &opaque->hand);

      // Update crafting system. While it is possible to store crafting_output
      // inside opaque struct, it is not necessary since we recompute it every frame
      // anyway.
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

    if(opaque->ui_state == PLAYER_UI_STATE_CONTAINER_OPENED)
    {
      // Container
      for(unsigned y=0; y<opaque->container->height; ++y)
        for(unsigned x=0; x<opaque->container->width; ++x)
          update_slot(ui_grid_get_rect_at(ui_layout.container, x, y), &opaque->container->items[y * opaque->container->width + x], &opaque->hand);
    }

    // Drop items.
    if(opaque->ui_state != PLAYER_UI_STATE_DEFAULT && opaque->hand.id != ITEM_NONE)
    {
      bool can_drop_item = true;

      // Hot bar
      {
        struct ui_rect rect = ui_grid_get_rect_total(ui_layout.hot_bar);
        if(ui_button(rect.position, rect.dimension))
          can_drop_item = false;
      }

      // Inventory
      {
        struct ui_rect rect = ui_grid_get_rect_total(ui_layout.inventory);
        if(ui_button(rect.position, rect.dimension))
          can_drop_item = false;
      }

      if(opaque->ui_state == PLAYER_UI_STATE_INVENTORY_OPENED)
      {
        // Crafting inputs
        {
          struct ui_rect rect = ui_grid_get_rect_total(ui_layout.crafting_inputs);
          if(ui_button(rect.position, rect.dimension))
            can_drop_item = false;
        }

        // Crafting output
        {
          struct ui_rect rect = ui_grid_get_rect_total(ui_layout.crafting_output);
          if(ui_button(rect.position, rect.dimension))
            can_drop_item = false;
        }
      }

      if(opaque->ui_state == PLAYER_UI_STATE_CONTAINER_OPENED)
      {
        // Container
        {
          struct ui_rect rect = ui_grid_get_rect_total(ui_layout.container);
          if(ui_button(rect.position, rect.dimension))
            can_drop_item = false;
        }
      }

      if(can_drop_item && input_press(BUTTON_LEFT))
      {
        const fvec3_t spawn_position = entity->position;
        const fvec3_t spawn_roation = fvec3_zero();
        const fvec3_t spawn_velocity = fvec3_zero();

        struct entity item_entity;
        entity_init(&item_entity, spawn_position, spawn_roation, spawn_velocity, INFINITY, INFINITY);
        item_entity_init(&item_entity, opaque->hand);
        world_add_entity(item_entity);

        opaque->hand.id = ITEM_NONE;
        opaque->hand.count = 0;
      }
    }
  }

  // Render
  {
    // Hot bar
    render_background(ui_grid_get_rect_total(ui_layout.hot_bar), PLAYER_HOTBAR_UI_COLOR_BACKGROUND);
    for(unsigned i=0; i<PLAYER_HOTBAR_SIZE; ++i)
      render_slot(ui_grid_get_rect_at(ui_layout.hot_bar, i, 0), PLAYER_HOTBAR_UI_COLOR_DEFAULT, PLAYER_HOTBAR_UI_COLOR_HOVER, PLAYER_HOTBAR_UI_COLOR_SELECTED, opaque->hotbar[i], i == opaque->hotbar_selection);

    if(opaque->ui_state != PLAYER_UI_STATE_DEFAULT)
    {
      // Inventory
      render_background(ui_grid_get_rect_total(ui_layout.inventory), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(unsigned y=0; y<PLAYER_INVENTORY_SIZE_VERTICAL; ++y)
        for(unsigned x=0; x<PLAYER_INVENTORY_SIZE_HORIZONTAL; ++x)
          render_slot(ui_grid_get_rect_at(ui_layout.inventory, x, y), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), opaque->inventory[y][x], false);

      // Hand
      render_item(ui_layout.hand, opaque->hand);
    }

    if(opaque->ui_state == PLAYER_UI_STATE_INVENTORY_OPENED)
    {
      // Crafting inputs
      render_background(ui_grid_get_rect_total(ui_layout.crafting_inputs), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(unsigned y=0; y<3; ++y)
        for(unsigned x=0; x<3; ++x)
          render_slot(ui_grid_get_rect_at(ui_layout.crafting_inputs, x, y), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), opaque->crafting_inputs[y][x], false);

      // Crafting output
      render_background(ui_grid_get_rect_total(ui_layout.crafting_output), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      render_slot(ui_grid_get_rect_at(ui_layout.crafting_output, 0, 0), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), crafting_output, false);
    }

    if(opaque->ui_state == PLAYER_UI_STATE_CONTAINER_OPENED)
    {
      // Container
      render_background(ui_grid_get_rect_total(ui_layout.container), PLAYER_INVENTORY_UI_COLOR_BACKGROUND);
      for(unsigned y=0; y<opaque->container->height; ++y)
        for(unsigned x=0; x<opaque->container->width; ++x)
          render_slot(ui_grid_get_rect_at(ui_layout.container, x, y), PLAYER_INVENTORY_UI_COLOR_DEFAULT, PLAYER_INVENTORY_UI_COLOR_HOVER, fvec4_zero(), opaque->container->items[y * opaque->container->width + x], false);
    }

    if(opaque->ui_state == PLAYER_UI_STATE_DEFAULT)
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
