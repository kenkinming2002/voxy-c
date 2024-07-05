#include <voxy/scene/main_game/entity/player.h>

#include <voxy/scene/main_game/types/entity.h>
#include <voxy/scene/main_game/types/item.h>

#include <voxy/scene/main_game/entity/weird.h>

#include <voxy/scene/main_game/render/assets.h>
#include <voxy/scene/main_game/config.h>

#include <voxy/scene/main_game/states/chunks.h>
#include <voxy/scene/main_game/states/seed.h>
#include <voxy/scene/main_game/states/camera.h>

#include <voxy/scene/main_game/mod.h>

#include <voxy/scene/main_game/update/generate.h>
#include <voxy/scene/main_game/update/chunk_generate.h>

#include <voxy/scene/main_game/render/debug.h>

#include <voxy/graphics/camera.h>
#include <voxy/graphics/gl.h>
#include <voxy/graphics/ui.h>

#include <voxy/core/window.h>
#include <voxy/core/log.h>

#include <voxy/utils.h>

#include <stdlib.h>

#define INVENTORY_SIZE_HORIZONTAL 9
#define INVENTORY_SIZE_VERTICAL   5

#define HOTBAR_SIZE 9

static inline int mini(int a, int b) { return a < b ? a : b; }
static inline float minf(float a, float b) { return a < b ? a : b; }

struct inventory
{
  struct item items[INVENTORY_SIZE_VERTICAL][INVENTORY_SIZE_HORIZONTAL];
};

struct hotbar
{
  struct item items[HOTBAR_SIZE];
  uint8_t selection;
};

struct player_opaque
{
  struct inventory inventory;
  struct hotbar    hotbar;
  struct item      hand;

  bool third_person;
  bool inventory_opened;

  float cooldown;
  float cooldown_weird;
};

static void player_entity_update(struct entity *entity, float dt);
static entity_id_t player_entity_id(void)
{
  static entity_id_t id = ENTITY_NONE;
  if(id == ENTITY_NONE)
  {
    struct entity_info entity_info;
    entity_info.mod = "main";
    entity_info.name = "player";
    entity_info.hitbox_dimension = fvec3(1.0f, 1.0f, 2.0f);
    entity_info.hitbox_offset = fvec3(0.0f, 0.0f, -0.5f);
    entity_info.on_update = player_entity_update;
    id = register_entity_info(entity_info);
  }
  return id;
}

void player_entity_init(struct entity *entity)
{
  struct player_opaque *opaque = malloc(sizeof *opaque);

  for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
    for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
    {
      opaque->inventory.items[j][i].id = ITEM_NONE;
      opaque->inventory.items[j][i].count = 0;
    }

  for(int i=0; i<HOTBAR_SIZE; ++i)
  {
    opaque->hotbar.items[i].id = ITEM_NONE;
    opaque->hotbar.items[i].count = 0;
  }

  opaque->hand.id = ITEM_NONE;
  opaque->hand.count = 0;

  int count = mini(5, HOTBAR_SIZE);
  for(int i=0; i<count; ++i)
  {
    opaque->hotbar.items[i].id = i;
    opaque->hotbar.items[i].count = 8 * (i + 1);
  }

  opaque->third_person = false;
  opaque->inventory_opened = false;
  opaque->cooldown = 0.0f;
  opaque->cooldown_weird = 0.0f;

  entity->id = player_entity_id();
  entity->opaque = opaque;
}

void player_entity_fini(struct entity *entity)
{
  free(entity->opaque);
}

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
  ui_quad_textured(cell->position, cell->dimension, 0.0f, texture.id);

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
  ui_text(position, label->height, label->text);
}

static void player_entity_update_ui(struct entity *entity)
{
  struct player_opaque *opaque = entity->opaque;

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
    opaque->inventory_opened = !opaque->inventory_opened;
    window_show_cursor(opaque->inventory_opened);
  }

  if(input_press(KEY_1)) opaque->hotbar.selection = 0;
  if(input_press(KEY_2)) opaque->hotbar.selection = 1;
  if(input_press(KEY_3)) opaque->hotbar.selection = 2;
  if(input_press(KEY_4)) opaque->hotbar.selection = 3;
  if(input_press(KEY_5)) opaque->hotbar.selection = 4;
  if(input_press(KEY_6)) opaque->hotbar.selection = 5;
  if(input_press(KEY_7)) opaque->hotbar.selection = 6;
  if(input_press(KEY_8)) opaque->hotbar.selection = 7;
  if(input_press(KEY_9)) opaque->hotbar.selection = 8;

  opaque->hotbar.selection += mouse_scroll.x;
  opaque->hotbar.selection += 9;
  opaque->hotbar.selection %= 9;

  /*********************
   * Major UI Handling *
   *********************/
  struct cell_widget cell_widget;
  cell_widget.dimension = cell_dimension;
  cell_widget.rounding  = rounding;
  cell_widget.hand      = &opaque->hand;

  ui_quad_colored(hotbar_position, hotbar_dimension, rounding, UI_HOTBAR_COLOR_BACKGROUND);
  for(int i=0; i<HOTBAR_SIZE; ++i)
  {
    cell_widget.position        = fvec2_add(fvec2_add(hotbar_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, 0), cell_sep + cell_width));
    cell_widget.default_color   = opaque->hotbar.selection == i ? UI_HOTBAR_COLOR_SELECTED : UI_HOTBAR_COLOR_DEFAULT;
    cell_widget.highlight_color = UI_HOTBAR_COLOR_HOVER;
    cell_widget.item            = &opaque->hotbar.items[i];

    cell_widget_render_background(&cell_widget);
    cell_widget_render_item(&cell_widget);
    if(opaque->inventory_opened)
      cell_widget_update(&cell_widget);
  }

  if(opaque->inventory_opened)
  {
    ui_quad_colored(inventory_position, inventory_dimension, rounding, UI_INVENTORY_COLOR_BACKGROUND);
    for(int j=0; j<INVENTORY_SIZE_VERTICAL; ++j)
      for(int i=0; i<INVENTORY_SIZE_HORIZONTAL; ++i)
      {
        cell_widget.position        = fvec2_add(fvec2_add(inventory_position, fvec2(cell_sep, cell_sep)), fvec2_mul_scalar(fvec2(i, j), cell_sep + cell_width));
        cell_widget.default_color   = UI_INVENTORY_COLOR_DEFAULT;
        cell_widget.highlight_color = UI_INVENTORY_COLOR_HOVER;
        cell_widget.item            = &opaque->inventory.items[j][i];

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
      gl_texture_2d_load(&cursor, "assets/textures/cursor.png");
      cursor_loaded = true;
    }

    fvec2_t dimension = fvec2(32.0f, 32.0f);
    fvec2_t position  = fvec2_mul_scalar(fvec2_sub(ivec2_as_fvec2(window_size), dimension), 0.5f);
    ui_quad_textured(position, dimension, 0.0f, cursor.id);
  }

  if(!opaque->inventory_opened)
  {
    struct label_widget label_widget;
    label_widget.height = UI_TEXT_SIZE;

    const struct item *selected_item = &opaque->hotbar.items[opaque->hotbar.selection];
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
      const struct block *target_block = world_get_block(position);
      if(target_block && target_block->id != BLOCK_NONE)
      {
        label_widget.text     = query_block_info(target_block->id)->name;
        label_widget.position = target_block_label_position;
        label_widget_render(&label_widget);
      }
    }
  }
}

static void player_entity_update_actions(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  opaque->cooldown += dt;
  if(!opaque->inventory_opened)
  {
    // Destroy block
    if(input_state(BUTTON_LEFT) && opaque->cooldown >= PLAYER_ACTION_COOLDOWN)
    {
      ivec3_t position;
      ivec3_t normal;
      if(entity_ray_cast(entity, 20.0f, &position, &normal))
      {
        int radius = input_state(KEY_CTRL) ? 2 : 0;
        for(int dz=-radius; dz<=radius; ++dz)
          for(int dy=-radius; dy<=radius; ++dy)
            for(int dx=-radius; dx<=radius; ++dx)
            {
              ivec3_t offset = ivec3(dx, dy, dz);
              if(ivec3_length_squared(offset) <= radius * radius)
              {
                ivec3_t block_position = ivec3_add(position, offset);

                struct block *block = world_get_block(block_position);
                block->id = 0;
                world_invalidate_block(block_position);
              }
            }
      }

      opaque->cooldown = 0.0f;
      return;
    }

    // Use item
    if(input_state(BUTTON_RIGHT) && opaque->cooldown >= PLAYER_ACTION_COOLDOWN)
    {
      struct item *item = &opaque->hotbar.items[opaque->hotbar.selection];
      if(item->id != ITEM_NONE)
      {
        const struct item_info *item_info = query_item_info(item->id);
        if(item_info->on_use)
          item_info->on_use(entity, item);
      }

      opaque->cooldown = 0.0f;
      return;
    }
  }
}

static void player_entity_update_controls(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  if(!opaque->inventory_opened)
  {
    // Camera - Third person
    opaque->third_person = opaque->third_person != input_press(KEY_F);

    // Camera - Rotation
    entity->rotation.yaw   +=  mouse_motion.x * PLAYER_PAN_SPEED;
    entity->rotation.pitch += -mouse_motion.y * PLAYER_PAN_SPEED;

    // World Camera
    world_camera.transform.translation = entity->position;
    world_camera.transform.rotation = entity->rotation;
    if(opaque->third_person)
      world_camera.transform.translation = fvec3_sub(world_camera.transform.translation, entity_local_to_global(entity, fvec3(0.0f, 10.0f, 0.0f)));

    world_camera.fovy   = M_PI / 2.0f;
    world_camera.near   = 0.1f;
    world_camera.far    = 1000.0f;
    world_camera.aspect = (float)window_size.x / (float)window_size.y;

    // Movement
    bool is_moving = false;
    fvec2_t direction = fvec2_zero();
    if(input_state(KEY_A)) { is_moving = true; direction.x -= 1.0f; }
    if(input_state(KEY_D)) { is_moving = true; direction.x += 1.0f; }
    if(input_state(KEY_S)) { is_moving = true; direction.y -= 1.0f; }
    if(input_state(KEY_W)) { is_moving = true; direction.y += 1.0f; }
    if(is_moving)
    {
      const float speed = entity->grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR;
      entity_move(entity, direction, speed, dt);
    }
    else if(entity->grounded)
    {
      // Player should actively stop their movement if they are not moving.
      const float speed = minf(dt * (entity->grounded ? PLAYER_MOVE_SPEED_GROUND : PLAYER_MOVE_SPEED_AIR), fvec3_length(entity->velocity));
      entity_apply_impulse(entity, fvec3_mul_scalar(fvec3_normalize(entity->velocity), -speed));
    }

    // Jump
    if(input_state(KEY_SPACE))
      entity_jump(entity, PLAYER_JUMP_STRENGTH);
  }
}

static void player_entity_update_weird(struct entity *entity, float dt)
{
  struct player_opaque *opaque = entity->opaque;
  for(opaque->cooldown_weird += dt; opaque->cooldown >= 2.0f; opaque->cooldown -= 2.0f)
  {
    struct entity new_entity;
    new_entity.position = entity->position;
    new_entity.velocity = entity->velocity;
    new_entity.rotation = entity->rotation;
    new_entity.grounded = entity->grounded;
    weird_entity_init(&new_entity);
    if(!world_add_entity(new_entity))
      weird_entity_fini(&new_entity);
  }
}

static void player_entity_update_load_chunks(struct entity *entity)
{
  ivec3_t center = fvec3_as_ivec3_floor(fvec3_div_scalar(entity->position, CHUNK_WIDTH));
  for(int dz = -GENERATOR_DISTANCE_PLAYER; dz<=GENERATOR_DISTANCE_PLAYER; ++dz)
    for(int dy = -GENERATOR_DISTANCE_PLAYER; dy<=GENERATOR_DISTANCE_PLAYER; ++dy)
      for(int dx = -GENERATOR_DISTANCE_PLAYER; dx<=GENERATOR_DISTANCE_PLAYER; ++dx)
        enqueue_chunk_generate(ivec3_add(center, ivec3(dx, dy, dz)));
}

static void player_entity_update(struct entity *entity, float dt)
{
  player_entity_update_ui(entity);
  player_entity_update_actions(entity, dt);
  player_entity_update_controls(entity, dt);
  player_entity_update_weird(entity, dt);
  player_entity_update_load_chunks(entity);

  if(input_press(KEY_P))
    g_render_debug = !g_render_debug;
}

