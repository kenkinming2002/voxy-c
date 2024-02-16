#include <voxy/main_game/player.h>
#include <voxy/main_game/mod.h>
#include <voxy/main_game/mod_assets.h>
#include <voxy/main_game/world_seed.h>
#include <voxy/main_game/world.h>

#include <voxy/types/entity.h>
#include <voxy/types/inventory.h>
#include <voxy/types/hotbar.h>

#include <voxy/graphics/gl.h>
#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>

#include <voxy/utils.h>
#include <voxy/config.h>

#include <stdio.h>
#include <stdlib.h>

static inline int mini(int a, int b) { return a < b ? a : b; }
static inline float minf(float a, float b) { return a < b ? a : b; }

struct player
{
  struct entity entity;

  struct inventory inventory;
  struct hotbar    hotbar;
  struct item      hand;

  bool third_person;
  bool inventory_opened;

  float cooldown;
};

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

  struct gl_texture_2d *texture = mod_assets_item_texture_get(cell->item->id);
  ui_quad_textured(cell->position, cell->dimension, 0.0f, texture->id);

  char buffer[4];
  snprintf(buffer, sizeof buffer, "%u", cell->item->count);
  ui_text(cell->position, UI_TEXT_SIZE_ITEM_COUNT, buffer);
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
  ui_text(position, label->height, label->text);
}

static void player_update_ui(struct player *player)
{
  /**************************************************
   * Laying out UI elements manually => How Lovely? *
   **************************************************/
  const float base       = minf(window_size.x, window_size.y);
  const float margin     = base * 0.008f;
  const float cell_width = base * 0.05f;
  const float cell_sep   = base * 0.006f;
  const float rounding   = base * 0.006f;

  const float   hotbar_width     = cell_sep + HOTBAR_SIZE * (cell_sep + cell_width);
  const float   hotbar_height    = 2 * cell_sep + cell_width;
  const fvec2_t hotbar_position  = fvec2((window_size.x - hotbar_width) * 0.5f, margin);
  const fvec2_t hotbar_dimension = fvec2(hotbar_width, hotbar_height);

  const float   inventory_width     = cell_sep + INVENTORY_SIZE_HORIZONTAL * (cell_sep + cell_width);
  const float   inventory_height    = cell_sep + INVENTORY_SIZE_VERTICAL   * (cell_sep + cell_width);
  const fvec2_t inventory_position  = fvec2((window_size.x - inventory_width) * 0.5f, (window_size.y - inventory_height) * 0.5f);
  const fvec2_t inventory_dimension = fvec2(inventory_width, inventory_height);

  const fvec2_t cell_dimension = fvec2(cell_width, cell_width);

  const fvec2_t selected_item_label_position = fvec2(window_size.x * 0.5f, hotbar_position.y + hotbar_dimension.y + margin);
  const fvec2_t target_block_label_position  = fvec2(window_size.x * 0.5f, window_size.y - margin - UI_TEXT_SIZE);

  /********************************************
   * Some basic input handling for UI element *
   ********************************************/
  if(input_press(KEY_I))
  {
    player->inventory_opened = !player->inventory_opened;
    window_show_cursor(player->inventory_opened);
  }

  if(input_press(KEY_1)) player->hotbar.selection = 0;
  if(input_press(KEY_2)) player->hotbar.selection = 1;
  if(input_press(KEY_3)) player->hotbar.selection = 2;
  if(input_press(KEY_4)) player->hotbar.selection = 3;
  if(input_press(KEY_5)) player->hotbar.selection = 4;
  if(input_press(KEY_6)) player->hotbar.selection = 5;
  if(input_press(KEY_7)) player->hotbar.selection = 6;
  if(input_press(KEY_8)) player->hotbar.selection = 7;
  if(input_press(KEY_9)) player->hotbar.selection = 8;

  player->hotbar.selection += mouse_scroll.x;
  player->hotbar.selection += 9;
  player->hotbar.selection %= 9;

  /*********************
   * Major UI Handling *
   *********************/
  struct cell_widget cell_widget;
  cell_widget.dimension = cell_dimension;
  cell_widget.rounding  = rounding;
  cell_widget.hand      = &player->hand;

  ui_quad_colored(hotbar_position, hotbar_dimension, rounding, UI_HOTBAR_COLOR_BACKGROUND);
  for(int i=0; i<HOTBAR_SIZE; ++i)
  {
    cell_widget.position        = fvec2_add(fvec2_add(hotbar_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, 0), cell_sep + cell_width));
    cell_widget.default_color   = player->hotbar.selection == i ? UI_HOTBAR_COLOR_SELECTED : UI_HOTBAR_COLOR_DEFAULT;
    cell_widget.highlight_color = UI_HOTBAR_COLOR_HOVER;
    cell_widget.item            = &player->hotbar.items[i];

    cell_widget_render_background(&cell_widget);
    cell_widget_render_item(&cell_widget);
    if(player->inventory_opened)
      cell_widget_update(&cell_widget);
  }

  if(player->inventory_opened)
  {
    ui_quad_colored(inventory_position, inventory_dimension, rounding, UI_INVENTORY_COLOR_BACKGROUND);
    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        cell_widget.position        = fvec2_add(fvec2_add(inventory_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, j), cell_sep + cell_width));
        cell_widget.default_color   = UI_INVENTORY_COLOR_DEFAULT;
        cell_widget.highlight_color = UI_INVENTORY_COLOR_HOVER;
        cell_widget.item            = &player->inventory.items[j][i];

        cell_widget_render_background(&cell_widget);
        cell_widget_render_item(&cell_widget);
        cell_widget_update(&cell_widget);
      }
  }

  if(player->inventory_opened)
  {
    cell_widget.position = fvec2_sub(mouse_position, fvec2_mul_scalar(fvec2(cell_width, cell_width), 0.5f));
    cell_widget.item     = &player->hand;

    cell_widget_render_item(&cell_widget);
  }

  if(!player->inventory_opened)
  {
    struct label_widget label_widget;
    label_widget.height = UI_TEXT_SIZE;

    const struct item *selected_item = &player->hotbar.items[player->hotbar.selection];
    if(selected_item->id != ITEM_NONE)
    {
      label_widget.text     = mod_item_info_get(selected_item->id)->name;
      label_widget.position = selected_item_label_position;
      label_widget_render(&label_widget);
    }

    ivec3_t position;
    ivec3_t normal;
    if(entity_ray_cast(&player->entity, 20.0f, &position, &normal))
    {
      const struct block *target_block = world_block_get(position);
      if(target_block && target_block->id != BLOCK_NONE)
      {
        label_widget.text     = mod_block_info_get(target_block->id)->name;
        label_widget.position = target_block_label_position;
        label_widget_render(&label_widget);
      }
    }
  }
}

static bool player_action(struct player *player)
{
  if(player->cooldown >= PLAYER_ACTION_COOLDOWN)
  {
    player->cooldown = 0.0f;
    return true;
  }
  else
    return false;
}

static void player_update_action(struct player *player, float dt)
{
  player->cooldown += dt;
  if(player->inventory_opened)
    return;

  if(input_state(BUTTON_LEFT) && player_action(player))
  {
    ivec3_t position;
    ivec3_t normal;
    if(entity_ray_cast(&player->entity, 20.0f, &position, &normal))
    {
      int radius = input_state(KEY_CTRL) ? 2 : 0;
      for(int dz=-radius; dz<=radius; ++dz)
        for(int dy=-radius; dy<=radius; ++dy)
          for(int dx=-radius; dx<=radius; ++dx)
          {
            ivec3_t offset = ivec3(dx, dy, dz);
            if(ivec3_length_squared(offset) <= radius * radius)
              world_block_set(ivec3_add(position, offset), 0);
          }
    }
  }

  if(input_state(BUTTON_RIGHT) && player_action(player))
  {
    const struct item *item = &player->hotbar.items[player->hotbar.selection];
    if(item->id != ITEM_NONE)
    {
      const struct item_info *item_info = mod_item_info_get(item->id);
      if(item_info->on_use)
        item_info->on_use(item->id);
    }
  }
}

static void player_update_camera(struct player *player)
{
  if(player->inventory_opened)
    return;

  player->third_person = player->third_person != input_press(KEY_F);

  fvec3_t rotation = fvec3_zero();
  rotation.yaw   =  mouse_motion.x * PLAYER_PAN_SPEED;
  rotation.pitch = -mouse_motion.y * PLAYER_PAN_SPEED;
  player->entity.local_view_transform = transform_rotate(player->entity.local_view_transform, rotation);
}

static void player_update_movement(struct player *player, float dt)
{
  if(player->inventory_opened)
    return;

  fvec2_t direction = fvec2_zero();
  if(input_state(KEY_A)) direction.x -= 1.0f;
  if(input_state(KEY_D)) direction.x += 1.0f;
  if(input_state(KEY_S)) direction.y -= 1.0f;
  if(input_state(KEY_W)) direction.y += 1.0f;
  entity_move(&player->entity, direction, player->entity.grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR, dt);

  if(input_state(KEY_SPACE))
    entity_jump(&player->entity, PLAYER_JUMP_STRENGTH);
}

static void player_update(struct entity *entity, float dt)
{
  struct player *player = container_of(entity, struct player, entity);
  player_update_ui(player);
  player_update_action(player, dt);
  player_update_camera(player);
  player_update_movement(player, dt);
}

static struct player *player;

struct player *player_get(void)
{
  return player;
}

struct entity *player_as_entity(struct player *player)
{
  return &player->entity;
}

bool player_third_person(struct player *player)
{
  return player->third_person;
}

void update_spawn_player(void)
{
  if(player)
    return;

  player = malloc(sizeof *player);

  player->entity.position                         = mod_generate_spawn(world_seed_get());
  player->entity.velocity                         = fvec3_zero();
  player->entity.dimension                        = PLAYER_DIMENSION;
  player->entity.local_view_transform.translation = fvec3(0.0f, 0.0f, PLAYER_EYE_HEIGHT);
  player->entity.local_view_transform.rotation    = fvec3(0.0f, 0.0f, 0.0f);
  player->entity.grounded                         = false;

  player->entity.update = &player_update;

  for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
    for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
    {
      player->inventory.items[j][i].id    = ITEM_NONE;
      player->inventory.items[j][i].count = 0;
    }

  for(int i=0; i<HOTBAR_SIZE; ++i)
  {
    player->hotbar.items[i].id    = ITEM_NONE;
    player->hotbar.items[i].count = 0;
  }

  player->hand.id    = ITEM_NONE;
  player->hand.count = 0;

  int count = mini(mod_item_info_count_get() * 2, HOTBAR_SIZE);
  for(int i=0; i<count; ++i)
  {
    player->hotbar.items[i].id = i % mod_item_info_count_get();
    player->hotbar.items[i].count = 8 * (i + 1);
  }

  player->third_person     = false;
  player->inventory_opened = false;

  player->cooldown = 0.0f;

  world_entity_add(&player->entity);
  fprintf(stderr, "INFO: Spawning player at (%f, %f, %f) with %d items\n", player->entity.position.x, player->entity.position.y, player->entity.position.z, count);
}

